#include "App.h"
#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlStateModifiers.h"
#include "GlStateStack.h"
#include "GlShaderCache.h"
#include "GlStaticMeshInstance.h"
#include "GlViewport.h"
#include "IGlMesh.h"

#include <algorithm>
#include <functional>

using namespace std::placeholders;

GlScene::GlScene()
	: m_lightColor(glm::vec4(1.f))
	, m_lightDirection(glm::vec3(0.f, 0.f, -1.f))
{
}

GlScene::~GlScene()
{
	removeAllInstances();
}

void GlScene::addInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material= instance->getMaterialInstanceConst()->getMaterial();

	if (m_drawCalls.find(material) == m_drawCalls.end())
	{
		m_drawCalls.insert({material, std::make_shared<GlDrawCall>()});
	}

	m_drawCalls[material]->instances.push_back(instance);
}

void GlScene::removeInstance(IGlSceneRenderableConstPtr instance)
{
	GlMaterialConstPtr material = instance->getMaterialInstanceConst()->getMaterial();

	auto drawCallIter= m_drawCalls.find(material);
	if (drawCallIter != m_drawCalls.end())
	{
		GlDrawCallPtr drawCall = drawCallIter->second;

		// Remove any existing instances from the draw call
		for (auto it = drawCall->instances.begin(); it != drawCall->instances.end(); it++)
		{
			IGlSceneRenderableConstPtr existingInstance= it->lock();

			if (existingInstance == instance)
			{
				drawCall->instances.erase(it);
				break;
			}
		}

		if (drawCall->instances.size() == 0)
		{
			m_drawCalls.erase(drawCallIter);
		}
	}
}

void GlScene::removeAllInstances()
{
	m_drawCalls.clear();
}

void GlScene::render(IMkCameraConstPtr camera, GlStateStack& glStateStack) const
{
	GlScopedState scopedState= glStateStack.createScopedState("GlScene");
	GlState& glState= scopedState.getStackState();

	// Enable front face culling while drawing the scene
	glState.enableFlag(eGlStateFlagType::cullFace);
	glStateSetFrontFace(glState, eGLFrontFaceMode::CCW);

	// Enable Depth Test while drawing the scene
	glState.enableFlag(eGlStateFlagType::depthTest);

	// Clear the depth buffer before drawing the scene
	glStateClearBuffer(glState, eGlClearFlags::depth);

	for (auto drawCallIter= m_drawCalls.begin(); drawCallIter != m_drawCalls.end(); drawCallIter++)
	{
		GlMaterialConstPtr material = drawCallIter->first;
		GlDrawCallConstPtr drawCall = drawCallIter->second;

		bool bAnyInstancesVisible= false;
		for (auto instanceIter = drawCall->instances.begin();
			 instanceIter != drawCall->instances.end();
			 instanceIter++)
		{
			IGlSceneRenderableConstPtr renderableInstance = instanceIter->lock();

			if (renderableInstance != nullptr && 
				renderableInstance->getVisible() && 
				renderableInstance->canCameraSee(camera))
			{
				bAnyInstancesVisible= true;
				break;
			}
		}

		if (bAnyInstancesVisible)
		{
			// Bind material program.
			// Unbound when materialBinding goes out of scope.
			auto materialBinding = 
				material->bindMaterial(
					// Scene specific material binding callback for camera and scene parameters
					[this, camera](
						GlProgramPtr program,
						eUniformDataType uniformDataType,
						eUniformSemantic uniformSemantic,
						const std::string& uniformName)
					{
						return materialBindCallback(
							camera, 
							program, uniformDataType, uniformSemantic, uniformName);
					});
			if (materialBinding)
			{
				for (auto instanceIter = drawCall->instances.begin();
					 instanceIter != drawCall->instances.end();
					 instanceIter++)
				{
					IGlSceneRenderableConstPtr renderableInstance = instanceIter->lock();

					if (renderableInstance != nullptr && 
						renderableInstance->getVisible() &&
						renderableInstance->canCameraSee(camera))
					{
						GlMaterialInstanceConstPtr materialInstance = renderableInstance->getMaterialInstanceConst();

						// Bind material instance parameters 
						// Unbound when materialInstanceBinding goes out of scope.
						auto materialInstanceBinding =
							materialInstance->bindMaterialInstance(
								materialBinding,
								[this, camera, renderableInstance](
									GlProgramPtr program,
									eUniformDataType uniformDataType,
									eUniformSemantic uniformSemantic,
									const std::string& uniformName) 
								{
									return materialInstanceBindCallback(
										camera, renderableInstance, 
										program, uniformDataType, uniformSemantic, uniformName);
								});

						if (materialInstanceBinding)
						{
							// Draw the renderable
							renderableInstance->render();
						}
					}
				}
			}
		}
	}
}

eUniformBindResult GlScene::materialBindCallback(
	IMkCameraConstPtr camera,
	GlProgramPtr program,
	eUniformDataType uniformDataType,
	eUniformSemantic uniformSemantic,
	const std::string& uniformName) const
{
	eUniformBindResult bindResult= eUniformBindResult::unbound;

	switch (uniformSemantic)
	{
	case eUniformSemantic::lightColorRGB:
		{
			bindResult =
				program->setVector3Uniform(uniformName, getLightColor())
				? eUniformBindResult::bound
				: eUniformBindResult::error;
		} break;
	case eUniformSemantic::lightDirection:
		{
			bindResult =
				program->setVector3Uniform(uniformName, getLightDirection())
				? eUniformBindResult::bound
				: eUniformBindResult::error;
		} break;
	case eUniformSemantic::viewMatrix:
		{
			if (camera != nullptr)
			{
				bindResult =
					program->setMatrix4x4Uniform(uniformName, camera->getViewMatrix())
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			else
			{
				bindResult = eUniformBindResult::error;
			}
		} break;
	case eUniformSemantic::projectionMatrix:
		{
			if (camera != nullptr)
			{
				bindResult =
					program->setMatrix4x4Uniform(uniformName, camera->getProjectionMatrix())
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			else
			{
				bindResult = eUniformBindResult::error;
			}
		} break;
	}

	return bindResult;
}

eUniformBindResult GlScene::materialInstanceBindCallback(
	IMkCameraConstPtr camera,
	IGlSceneRenderableConstPtr renderableInstance,
	GlProgramPtr program,
	eUniformDataType uniformDataType,
	eUniformSemantic uniformSemantic,
	const std::string& uniformName) const
{
	eUniformBindResult bindResult= eUniformBindResult::unbound;

	switch (uniformSemantic)
	{
		case eUniformSemantic::modelMatrix:
			{
				const glm::mat4 modelMat = renderableInstance->getModelMatrix();

				bindResult= 
					program->setMatrix4x4Uniform(uniformName, modelMat)
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			break;
		case eUniformSemantic::normalMatrix:
			{
				const glm::mat4 normalMat = renderableInstance->getNormalMatrix();

				bindResult= 
					program->setMatrix4x4Uniform(uniformName, normalMat)
					? eUniformBindResult::bound
					: eUniformBindResult::error;
			}
			break;
		case eUniformSemantic::modelViewProjectionMatrix:
			{
				if (camera != nullptr)
				{
					const glm::mat4 viewProjMat = camera->getViewProjectionMatrix();
					const glm::mat4 modelMat = renderableInstance->getModelMatrix();
					const glm::mat4 modelViewProjMatrix = viewProjMat * modelMat;

					bindResult= 
						program->setMatrix4x4Uniform(uniformName, modelViewProjMatrix)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
				}
				else
				{
					bindResult = eUniformBindResult::error;
				}
			}
			break;
	}

	return bindResult;
}