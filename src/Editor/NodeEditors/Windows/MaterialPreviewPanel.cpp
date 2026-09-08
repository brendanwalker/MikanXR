//-- includes -----
#include "MaterialPreviewPanel.h"
#include "IMkFrameBuffer.h"
#include "IMkGraphicsContext.h"
#include "IMkShader.h"
#include "IMkShaderCache.h"
#include "IMkShaderCode.h"
#include "IMkState.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "IMkVertexDefinition.h"
#include "LocText.h"
#include "Logger.h"
#include "MikanModelResourceManager.h"
#include "MikanRenderModelResource.h"
#include "MikanTextureCache.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkScopedObjectBinding.h"
#include "MkScopedState.h"
#include "MkStateModifiers.h"
#include "MkStateStack.h"
#include "PathUtils.h"

#include "imgui.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <algorithm>
#include <cmath>

namespace
{
const int k_materialPreviewSize= 256;
const float k_materialPreviewOrbitRadiansPerSecond= 0.5f;
const float k_materialPreviewOrbitElevationRadians= 0.35f;
// Fits the radius 0.5 sphere and the unit quad under a 45 degree vertical fov
const float k_materialPreviewOrbitDistance= 1.6f;
const float k_materialPreviewFovRadians= 0.7854f;
const float k_materialPreviewZNear= 0.1f;
const float k_materialPreviewZFar= 10.f;
const glm::vec4 k_materialPreviewClearColor(0.18f, 0.18f, 0.18f, 1.f);
const char* k_materialPreviewProgramName= "__preview_material_program";
const char* k_materialPreviewMaterialName= "__preview_material";
const char* k_materialPreviewTimeParameterName= "time";

eUniformSemantic materialPreviewSemanticFromName(const std::string& semanticName)
{
	for (int enumIntValue= 0; enumIntValue < (int)eUniformSemantic::COUNT; ++enumIntValue)
	{
		const eUniformSemantic semantic= (eUniformSemantic)enumIntValue;
		if (getUniformSemanticName(semantic) == semanticName)
		{
			return semantic;
		}
	}

	return eUniformSemantic::INVALID;
}

const MaterialParameterDefault* materialPreviewFindParameter(const std::vector<MaterialParameterDefault>& defaults,
															 const std::string& name)
{
	for (const MaterialParameterDefault& parameterDefault : defaults)
	{
		if (parameterDefault.name == name)
		{
			return &parameterDefault;
		}
	}

	return nullptr;
}

std::string materialPreviewJoinUniformNames(const UniformNameSet& uniformNames)
{
	std::string joined;
	for (const std::string& uniformName : uniformNames)
	{
		if (!joined.empty())
		{
			joined+= ", ";
		}
		joined+= uniformName;
	}

	return joined;
}

eUniformBindResult materialPreviewBindTexture(IMkShaderPtr program, const std::string& uniformName,
											  IMkTexturePtr texture)
{
	int textureUnit= 0;
	if (!texture || !program->getUniformTextureUnit(uniformName, textureUnit))
	{
		return eUniformBindResult::unbound;
	}

	return program->setTextureUniform(uniformName) && texture->bindTexture(textureUnit) ? eUniformBindResult::bound
																						: eUniformBindResult::error;
}
} // namespace

void MaterialPreviewPanel::setCompileResult(const MaterialCompileResult& result, eMaterialDomain domain,
											eMaterialVertexPreset preset)
{
	// A graph with errors has no source worth compiling. The canvas overlay
	// already shows the errors, so the previous good picture stays up.
	if (result.hasErrors())
	{
		return;
	}

	m_pendingResult= result;
	m_pendingDomain= domain;
	m_pendingPreset= preset;
	m_bHasPendingResult= true;
}

void MaterialPreviewPanel::render(IMkGraphicsContext* graphicsContext, MikanTextureCache* textureCache,
								  MikanModelResourceManager* modelResourceManager, float deltaSeconds)
{
	if (graphicsContext == nullptr || textureCache == nullptr || modelResourceManager == nullptr)
	{
		return;
	}

	if (m_bHasPendingResult)
	{
		m_bHasPendingResult= false;
		compilePendingResult(graphicsContext, modelResourceManager);
	}

	if (m_bParameterTexturesDirty)
	{
		refreshParameterTextures(textureCache);
	}

	updateOrbitCamera(deltaSeconds);

	if (!ensureFrameBuffer())
	{
		return;
	}

	drawPreview(graphicsContext, textureCache);
}

void MaterialPreviewPanel::renderUi(MkGuiStyleManager* styleManager)
{
	IMkTexturePtr colorTexture= m_frameBuffer ? m_frameBuffer->getColorTexture() : IMkTexturePtr();

	const bool bHasCompileLog= !m_compileLog.empty();
	const bool bHasUnboundStatus= !m_unboundUniformStatus.empty();

	// Square image filling the panel, leaving a few lines for status text when there is any
	ImVec2 available= ImGui::GetContentRegionAvail();
	if (bHasCompileLog || bHasUnboundStatus)
	{
		available.y-= ImGui::GetTextLineHeightWithSpacing() * 4.f;
	}
	const float side= std::max(1.f, std::min(available.x, available.y));

	if (colorTexture && colorTexture->getGlTextureId() != 0)
	{
		// The framebuffer texture is bottom-up, so flip V for the top-down image widget
		ImGui::Image((void*)(intptr_t)colorTexture->getGlTextureId(), ImVec2(side, side), ImVec2(0.f, 1.f),
					 ImVec2(1.f, 0.f));
	}
	else
	{
		ImGui::Dummy(ImVec2(side, side));
	}

	if (bHasCompileLog || bHasUnboundStatus)
	{
		MkGuiScopedStyle errorTextStyle(styleManager != nullptr ? styleManager->getStyle("node_editor_error_text")
																: MkGuiStyleConstPtr());
		if (bHasCompileLog)
		{
			ImGui::TextUnformatted(locText("materialEditor.compileLogHeader"));
			ImGui::TextWrapped("%s", m_compileLog.c_str());
		}
		if (bHasUnboundStatus)
		{
			const std::string status= locFormat("materialEditor.unboundUniformsFmt", m_unboundUniformStatus.c_str());
			ImGui::TextWrapped("%s", status.c_str());
		}
	}
}

void MaterialPreviewPanel::dispose()
{
	m_meshes.clear();
	m_sphereModel= nullptr;

	if (m_frameBuffer)
	{
		m_frameBuffer->disposeResources();
		m_frameBuffer= nullptr;
	}

	m_materialInstance= nullptr;
	m_material= nullptr;
	if (m_shader)
	{
		m_shader->deleteProgram();
		m_shader= nullptr;
	}

	m_parameterTextures.clear();
	m_parameterDefaults.clear();
	m_bHasPendingResult= false;
}

bool MaterialPreviewPanel::compilePendingResult(IMkGraphicsContext* graphicsContext,
												MikanModelResourceManager* modelResourceManager)
{
	const MaterialCompileResult& result= m_pendingResult;
	if (!result.config)
	{
		return false;
	}

	IMkShaderCodePtr shaderCode=
		createIMkShaderCode(k_materialPreviewProgramName, result.vertexSource, result.fragmentSource);

	for (const GlVertexAttributeConfigPtr& attributeConfig : result.config->vertexAttributes)
	{
		if (!attributeConfig || attributeConfig->dataType == eVertexDataType::INVALID
			|| attributeConfig->semantic == eVertexSemantic::INVALID)
		{
			m_compileLog= locText("materialEditor.previewInvalidAttribute");
			return false;
		}

		shaderCode->addVertexAttribute(attributeConfig->name, attributeConfig->dataType, attributeConfig->semantic);
	}

	for (const auto& [uniformName, semanticName] : result.config->uniformSemanticMap)
	{
		const eUniformSemantic semantic= materialPreviewSemanticFromName(semanticName);
		if (semantic == eUniformSemantic::INVALID)
		{
			m_compileLog=
				locFormat("materialEditor.previewUnknownSemanticFmt", semanticName.c_str(), uniformName.c_str());
			return false;
		}

		shaderCode->addUniform(uniformName, semantic);
	}

	IMkShaderPtr shader= createIMkShader(shaderCode);
	if (!shader || !shader->compileProgram())
	{
		// Keep the previous good material on screen alongside the driver's log
		m_compileLog= shader ? shader->getCompileLog() : std::string();
		if (m_compileLog.empty())
		{
			m_compileLog= locText("materialEditor.previewCompileFailed");
		}
		return false;
	}

	// The material only weakly references its program, so the panel owns it
	IMkShaderPtr previousShader= m_shader;
	m_shader= shader;
	m_material= std::make_shared<MkMaterial>(k_materialPreviewMaterialName, shader);
	m_materialInstance= createMkMaterialInstance(m_material);
	m_parameterDefaults= result.parameterDefaults;
	m_bParameterTexturesDirty= true;
	m_domain= m_pendingDomain;
	m_preset= m_pendingPreset;
	m_compileLog.clear();
	m_unboundUniformStatus.clear();

	if (previousShader)
	{
		previousShader->deleteProgram();
	}

	// The meshes bind their vertex layout to the material's program, so a new
	// program means new meshes even when the preset is unchanged
	return rebuildMeshes(graphicsContext, modelResourceManager);
}

bool MaterialPreviewPanel::rebuildMeshes(IMkGraphicsContext* graphicsContext,
										 MikanModelResourceManager* modelResourceManager)
{
	m_meshes.clear();

	if (!m_material)
	{
		return false;
	}

	switch (m_preset)
	{
	case eMaterialVertexPreset::P2T:
	{
		IMkTriangulatedMeshPtr quadMesh= createFullscreenQuadMesh(graphicsContext, m_material, false);
		if (quadMesh)
		{
			m_meshes.push_back(quadMesh);
		}
	}
	break;
	case eMaterialVertexPreset::PT:
		buildUnitQuadMesh(graphicsContext);
		break;
	case eMaterialVertexPreset::PNT:
		if (!buildSphereMeshes(graphicsContext, modelResourceManager))
		{
			if (!m_bLoggedSphereFallback)
			{
				MIKAN_LOG_WARNING("MaterialPreviewPanel::rebuildMeshes")
					<< "Failed to load the preview sphere model, falling back to a quad";
				m_bLoggedSphereFallback= true;
			}
			buildUnitQuadMesh(graphicsContext);
		}
		break;
	default:
		break;
	}

	return !m_meshes.empty();
}

bool MaterialPreviewPanel::buildUnitQuadMesh(IMkGraphicsContext* graphicsContext)
{
	// Unit quad in the XY plane, the same layout as the quad shape component
	struct MaterialPreviewPTVertex
	{
		float x, y, z, u, v;
	};
	static const MaterialPreviewPTVertex k_vertices[4]= {
		{-0.5f, -0.5f, 0.f, 0.f, 1.f},
		{0.5f, -0.5f, 0.f, 1.f, 1.f},
		{0.5f, 0.5f, 0.f, 1.f, 0.f},
		{-0.5f, 0.5f, 0.f, 0.f, 0.f},
	};
	static const uint16_t k_indices[6]= {0, 1, 2, 0, 2, 3};

	// A PNT fallback program cannot take this layout, so refuse rather than build a broken mesh
	IMkVertexDefinitionConstPtr vertexDefinition= m_material->getProgram()->getVertexDefinition();
	if (!vertexDefinition || vertexDefinition->getVertexSize() != sizeof(MaterialPreviewPTVertex))
	{
		return false;
	}

	IMkTriangulatedMeshPtr quadMesh= createMkTriangulatedMesh(
		graphicsContext, "material_preview_quad", reinterpret_cast<const uint8_t*>(k_vertices),
		sizeof(MaterialPreviewPTVertex), 4, reinterpret_cast<const uint8_t*>(k_indices), sizeof(uint16_t), 2, false);
	if (!quadMesh || !quadMesh->setMaterial(m_material) || !quadMesh->createResources())
	{
		return false;
	}

	m_meshes.push_back(quadMesh);
	return true;
}

bool MaterialPreviewPanel::buildSphereMeshes(IMkGraphicsContext* graphicsContext,
											 MikanModelResourceManager* modelResourceManager)
{
	// The sphere is imported once against the internal PNT material, whose
	// vertex layout is the PNT preset's, then re-wrapped in meshes bound to the
	// preview material so the import cache never keys on a throwaway material
	if (!m_sphereModel)
	{
		MkMaterialConstPtr importMaterial=
			graphicsContext->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED);
		if (!importMaterial)
		{
			return false;
		}

		const std::filesystem::path spherePath=
			PathUtils::makeAbsoluteResourceFilePath(std::filesystem::path("models") / "shapes" / "sphere.obj");
		m_sphereModel= modelResourceManager->fetchRenderModel(spherePath, importMaterial);
	}

	if (!m_sphereModel || m_sphereModel->getTriangulatedMeshCount() == 0)
	{
		return false;
	}

	IMkVertexDefinitionConstPtr vertexDefinition= m_material->getProgram()->getVertexDefinition();
	if (!vertexDefinition)
	{
		return false;
	}

	for (int meshIndex= 0; meshIndex < m_sphereModel->getTriangulatedMeshCount(); ++meshIndex)
	{
		IMkTriangulatedMeshPtr sourceMesh= m_sphereModel->getTriangulatedMesh(meshIndex);
		if (!sourceMesh || !sourceMesh->getMaterialInstance())
		{
			continue;
		}

		const size_t sourceVertexSize=
			sourceMesh->getMaterialInstance()->getMaterial()->getProgram()->getVertexDefinition()->getVertexSize();
		if (sourceVertexSize != vertexDefinition->getVertexSize() || sourceMesh->getVertexData() == nullptr
			|| sourceMesh->getIndexData() == nullptr)
		{
			continue;
		}

		// The source mesh keeps its buffers for its lifetime and m_sphereModel holds it
		IMkTriangulatedMeshPtr previewMesh=
			createMkTriangulatedMesh(graphicsContext, "material_preview_sphere", sourceMesh->getVertexData(),
									 sourceVertexSize, sourceMesh->getVertexCount(), sourceMesh->getIndexData(),
									 sourceMesh->getIndexSize(), (uint32_t)sourceMesh->getElementCount(), false);
		if (previewMesh && previewMesh->setMaterial(m_material) && previewMesh->createResources())
		{
			m_meshes.push_back(previewMesh);
		}
	}

	return !m_meshes.empty();
}

bool MaterialPreviewPanel::ensureFrameBuffer()
{
	if (m_frameBuffer && m_frameBuffer->isValid() && m_frameBuffer->getWidth() == k_materialPreviewSize
		&& m_frameBuffer->getHeight() == k_materialPreviewSize)
	{
		return true;
	}

	if (m_frameBuffer)
	{
		m_frameBuffer->disposeResources();
	}

	m_frameBuffer= createMkFrameBuffer("material_preview");
	m_frameBuffer->setFrameBufferType(IMkFrameBuffer::eFrameBufferType::COLOR_AND_DEPTH);
	m_frameBuffer->setColorFormat(IMkFrameBuffer::eColorFormat::RGBA);
	m_frameBuffer->setSize(k_materialPreviewSize, k_materialPreviewSize);
	m_frameBuffer->setClearColor(k_materialPreviewClearColor);

	if (!m_frameBuffer->createResources())
	{
		MIKAN_LOG_ERROR("MaterialPreviewPanel::ensureFrameBuffer") << "Failed to create the preview frame buffer";
		m_frameBuffer= nullptr;
		return false;
	}

	return true;
}

void MaterialPreviewPanel::refreshParameterTextures(MikanTextureCache* textureCache)
{
	m_parameterTextures.clear();

	for (const MaterialParameterDefault& parameterDefault : m_parameterDefaults)
	{
		if (parameterDefault.type != eShaderValueType::texture2D || parameterDefault.textureAssetPath.empty())
		{
			continue;
		}

		IMkTexturePtr texture= textureCache->loadTexturePath(parameterDefault.textureAssetPath);
		if (texture)
		{
			m_parameterTextures[parameterDefault.name]= texture;
		}
	}

	m_bParameterTexturesDirty= false;
}

void MaterialPreviewPanel::updateOrbitCamera(float deltaSeconds)
{
	m_accumulatedSeconds+= deltaSeconds;
	m_orbitAngleRadians=
		std::fmod(m_orbitAngleRadians + deltaSeconds * k_materialPreviewOrbitRadiansPerSecond, 6.2831853f);

	const float horizontalDistance= k_materialPreviewOrbitDistance * std::cos(k_materialPreviewOrbitElevationRadians);
	m_cameraPosition= glm::vec3(horizontalDistance * std::sin(m_orbitAngleRadians),
								k_materialPreviewOrbitDistance * std::sin(k_materialPreviewOrbitElevationRadians),
								horizontalDistance * std::cos(m_orbitAngleRadians));
	m_viewMatrix= glm::lookAt(m_cameraPosition, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	m_projectionMatrix=
		glm::perspective(k_materialPreviewFovRadians, 1.f, k_materialPreviewZNear, k_materialPreviewZFar);
}

void MaterialPreviewPanel::drawPreview(IMkGraphicsContext* graphicsContext, MikanTextureCache* textureCache)
{
	MkStateStack& stateStack= graphicsContext->getMkStateStack();
	MkScopedState scopedState= stateStack.createScopedState("MaterialPreviewPanel");
	IMkState* glState= scopedState.getStackState();

	// Binding clears the color and depth attachments to the clear color
	MkScopedObjectBinding frameBufferBinding(glState, "MaterialPreviewPanel", m_frameBuffer);
	if (!frameBufferBinding)
	{
		return;
	}

	// The binding pushed its own state, so the draw flags go on that one
	IMkState* drawState= frameBufferBinding.getMkState();
	if (m_domain == eMaterialDomain::shape)
	{
		drawState->enableFlag(eMkStateFlagType::depthTest);
		drawState->disableFlag(eMkStateFlagType::blend);
	}
	else
	{
		// The compositor layer quad composites over what is below it
		drawState->disableFlag(eMkStateFlagType::depthTest);
		drawState->enableFlag(eMkStateFlagType::blend);
		mkStateSetBlendEquation(drawState, eMkBlendEquation::ADD);
		mkStateSetBlendFunc(drawState, eMkBlendFunction::SRC_ALPHA, eMkBlendFunction::ONE_MINUS_SRC_ALPHA);
	}
	// The orbit sees the quad from behind half the time
	drawState->disableFlag(eMkStateFlagType::cullFace);

	if (!m_material || !m_materialInstance || m_meshes.empty())
	{
		return;
	}

	MkScopedMaterialBinding materialBinding= m_material->bindMaterial();
	if (!materialBinding)
	{
		m_unboundUniformStatus= materialPreviewJoinUniformNames(materialBinding.getUnboundUniforms());
		return;
	}

	const glm::mat4 mvpMatrix= m_projectionMatrix * m_viewMatrix;
	const glm::vec2 screenSize((float)m_frameBuffer->getWidth(), (float)m_frameBuffer->getHeight());
	IMkTexturePtr missingTexture= textureCache->tryGetTextureByName(INTERNAL_MISSING_TEXTURE_RGBA);

	BindUniformCallback bindCallback= [&](IMkShaderPtr program, eUniformDataType dataType, eUniformSemantic semantic,
										  const std::string& uniformName) -> eUniformBindResult
	{
		const auto boundOrError= [](bool bSuccess)
		{ return bSuccess ? eUniformBindResult::bound : eUniformBindResult::error; };

		switch (semantic)
		{
		case eUniformSemantic::modelViewProjectionMatrix:
			return boundOrError(program->setMatrix4x4Uniform(uniformName, mvpMatrix));
		case eUniformSemantic::modelMatrix:
		case eUniformSemantic::normalMatrix:
		case eUniformSemantic::transformMatrix:
			return boundOrError(program->setMatrix4x4Uniform(uniformName, glm::mat4(1.f)));
		case eUniformSemantic::viewMatrix:
			return boundOrError(program->setMatrix4x4Uniform(uniformName, m_viewMatrix));
		case eUniformSemantic::projectionMatrix:
			return boundOrError(program->setMatrix4x4Uniform(uniformName, m_projectionMatrix));
		case eUniformSemantic::screenSize:
			return boundOrError(program->setVector2Uniform(uniformName, screenSize));
		case eUniformSemantic::screenPosition:
			return boundOrError(program->setVector2Uniform(uniformName, glm::vec2(0.f)));
		case eUniformSemantic::cameraPosition:
			return boundOrError(program->setVector3Uniform(uniformName, m_cameraPosition));
		case eUniformSemantic::zNear:
			return boundOrError(program->setFloatUniform(uniformName, k_materialPreviewZNear));
		case eUniformSemantic::zFar:
			return boundOrError(program->setFloatUniform(uniformName, k_materialPreviewZFar));
		case eUniformSemantic::floatParam:
		{
			if (uniformName == k_materialPreviewTimeParameterName)
			{
				return boundOrError(program->setFloatUniform(uniformName, m_accumulatedSeconds));
			}

			const MaterialParameterDefault* parameter= materialPreviewFindParameter(m_parameterDefaults, uniformName);
			const float value= parameter ? parameter->value[0] : 0.f;
			return boundOrError(program->setFloatUniform(uniformName, value));
		}
		case eUniformSemantic::float2Param:
		{
			const MaterialParameterDefault* parameter= materialPreviewFindParameter(m_parameterDefaults, uniformName);
			const glm::vec2 value= parameter ? glm::make_vec2(parameter->value.data()) : glm::vec2(0.f);
			return boundOrError(program->setVector2Uniform(uniformName, value));
		}
		case eUniformSemantic::float3Param:
		{
			const MaterialParameterDefault* parameter= materialPreviewFindParameter(m_parameterDefaults, uniformName);
			const glm::vec3 value= parameter ? glm::make_vec3(parameter->value.data()) : glm::vec3(0.f);
			return boundOrError(program->setVector3Uniform(uniformName, value));
		}
		case eUniformSemantic::float4Param:
		{
			const MaterialParameterDefault* parameter= materialPreviewFindParameter(m_parameterDefaults, uniformName);
			const glm::vec4 value= parameter ? glm::make_vec4(parameter->value.data()) : glm::vec4(0.f);
			return boundOrError(program->setVector4Uniform(uniformName, value));
		}
		case eUniformSemantic::textureParam:
		{
			auto textureIter= m_parameterTextures.find(uniformName);
			IMkTexturePtr texture= textureIter != m_parameterTextures.end() ? textureIter->second : missingTexture;
			return materialPreviewBindTexture(program, uniformName, texture);
		}
		default:
			break;
		}

		// Every other texture input (layer textures, depth, distortion) shows the missing texture
		if (dataType == eUniformDataType::datatype_texture)
		{
			return materialPreviewBindTexture(program, uniformName, missingTexture);
		}

		return eUniformBindResult::unbound;
	};

	MkScopedMaterialInstanceBinding instanceBinding=
		m_materialInstance->bindMaterialInstance(materialBinding, bindCallback);
	if (instanceBinding)
	{
		m_unboundUniformStatus.clear();
		for (const IMkTriangulatedMeshPtr& mesh : m_meshes)
		{
			mesh->drawElements();
		}
	}
	else
	{
		m_unboundUniformStatus= materialPreviewJoinUniformNames(instanceBinding.getUnboundUniforms());
	}
}
