#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlProgram.h"
#include "GlRenderModelResource.h"
#include "GlShaderCache.h"
#include "GlStaticMeshInstance.h"
#include "GlTextureCache.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "IGlWindow.h"
#include "MathTypeConversion.h"
#include "BoxColliderComponent.h"
#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "StencilObjectSystem.h"

StencilObjectSystemWeakPtr StencilObjectSystem::s_stencilObjectSystem;

bool StencilObjectSystem::init()
{
	MikanObjectSystem::init();

	StencilObjectSystemConfigConstPtr stencilSystemConfig = getStencilSystemConfigConst();

	for (QuadStencilDefinitionPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		createQuadStencilObject(quadConfig);
	}

	for (BoxStencilDefinitionPtr boxConfig : stencilSystemConfig->boxStencilList)
	{
		createBoxStencilObject(boxConfig);
	}

	for (ModelStencilDefinitionPtr modelConfig : stencilSystemConfig->modelStencilList)
	{
		createModelStencilObject(modelConfig);
	}

	s_stencilObjectSystem = std::static_pointer_cast<StencilObjectSystem>(shared_from_this());
	return true;
}

void StencilObjectSystem::dispose()
{
	s_stencilObjectSystem.reset();
	m_quadStencilComponents.clear();
	m_boxStencilComponents.clear();
	m_modelStencilComponents.clear();

	MikanObjectSystem::dispose();
}

void StencilObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	QuadStencilComponentPtr quadStencil= objectPtr->getComponentOfType<QuadStencilComponent>();
	if (quadStencil != nullptr)
	{
		removeQuadStencil(quadStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}

	BoxStencilComponentPtr boxStencil = objectPtr->getComponentOfType<BoxStencilComponent>();
	if (boxStencil != nullptr)
	{
		removeBoxStencil(boxStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}

	ModelStencilComponentPtr modelStencil = objectPtr->getComponentOfType<ModelStencilComponent>();
	if (modelStencil != nullptr)
	{
		removeModelStencil(modelStencil->getStencilComponentDefinition()->getStencilId());
		return;
	}
}

StencilComponentPtr StencilObjectSystem::getStencilById(MikanStencilID stencilId) const
{
	QuadStencilComponentPtr quadPtr = getQuadStencilById(stencilId);
	if (quadPtr)
	{
		return quadPtr;
	}

	BoxStencilComponentPtr boxPtr = getBoxStencilById(stencilId);
	if (boxPtr)
	{
		return boxPtr;
	}

	ModelStencilComponentPtr modelPtr = getModelStencilById(stencilId);
	if (modelPtr)
	{
		return modelPtr;
	}

	return StencilComponentPtr();
}

eStencilType StencilObjectSystem::getStencilType(MikanStencilID stencilId) const
{
	QuadStencilComponentPtr quadPtr = getQuadStencilById(stencilId);
	if (quadPtr)
	{
		return eStencilType::quad;
	}

	BoxStencilComponentPtr boxPtr = getBoxStencilById(stencilId);
	if (boxPtr)
	{
		return eStencilType::box;
	}

	ModelStencilComponentPtr modelPtr = getModelStencilById(stencilId);
	if (modelPtr)
	{
		return eStencilType::model;
	}

	return eStencilType::INVALID;
}

bool StencilObjectSystem::getStencilWorldTransform(
	MikanStencilID parentStencilId, 
	glm::mat4& outXform) const
{
	StencilComponentPtr stencilComponent= getStencilById(parentStencilId);
	if (stencilComponent)
	{
		outXform = stencilComponent->getWorldTransform();
		return true;
	}

	return false;
}

bool StencilObjectSystem::removeStencil(MikanStencilID stencilId)
{
	switch (getStencilType(stencilId))
	{
	case eStencilType::quad:
		return removeQuadStencil(stencilId);
		break;
	case eStencilType::box:
		return removeBoxStencil(stencilId);
		break;
	case eStencilType::model:
		return removeModelStencil(stencilId);
		break;
	}

	return false;
}

void StencilObjectSystem::setRenderStencilsFlag(bool flag)
{
	// Update visibility on all model stencils
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr)
		{
			componentPtr->setRenderStencilsFlag(flag);
		}
	}

	// Forward flag on to the config
	getStencilSystemConfig()->setRenderStencilsFlag(flag);
}

QuadStencilComponentPtr StencilObjectSystem::getQuadStencilById(MikanStencilID stencilId) const
{
	auto iter = m_quadStencilComponents.find(stencilId);
	if (iter != m_quadStencilComponents.end())
	{
		return iter->second.lock();
	}

	return QuadStencilComponentPtr();
}

QuadStencilComponentPtr StencilObjectSystem::getQuadStencilByName(const std::string& stencilName) const
{
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		QuadStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return QuadStencilComponentPtr();
}

QuadStencilComponentPtr StencilObjectSystem::addNewQuadStencil(const MikanStencilQuad& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewQuadStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		QuadStencilDefinitionPtr configPtr = stencilSystemConfig->getQuadStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createQuadStencilObject(configPtr);
	}

	return QuadStencilComponentPtr();
}

bool StencilObjectSystem::removeQuadStencil(MikanStencilID stencilId)
{
	return
		disposeQuadStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantQuadStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<QuadStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_quadStencilComponents.begin(); it != m_quadStencilComponents.end(); it++)
	{
		MikanStencilID stencilId= it->first;
		QuadStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getStencilComponentDefinition()->getIsDisabled())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(allowedStencilIds->begin(), allowedStencilIds->end(), stencilId) == allowedStencilIds->end())
			{
				continue;
			}
		}

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

		{
			const glm::mat4 worldXform = componentPtr->getWorldTransform();
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilForward = glm::vec3(worldXform[2]); // forward is 2nd column
			const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;

			// Stencil is in front of the camera
			// Stencil is facing the camera (or double sided)
			if (glm::dot(cameraToStencil, cameraForward) > 0.f &&
				(componentPtr->getQuadStencilDefinition()->getIsDoubleSided() || glm::dot(stencilForward, cameraForward) < 0.f))
			{
				outStencilList.push_back(componentPtr);
			}
		}
	}
}

QuadStencilComponentPtr StencilObjectSystem::createQuadStencilObject(QuadStencilDefinitionPtr quadConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(quadConfig->getComponentName());

	// Make the QuadStencil component the root of the object
	QuadStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<QuadStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	stencilComponentPtr->setDefinition(quadConfig);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Attach a box collider to quad stencil component
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(quadConfig->getQuadWidth() * 0.5f, quadConfig->getQuadHeight() * 0.5f, 0.01f));
	boxColliderPtr->attachToComponent(stencilComponentPtr);

	// Init the object once all components are added
	stencilObject->init();

	// Keep track of all the quad stencils in the stencil system
	m_quadStencilComponents.insert({quadConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeQuadStencilObject(MikanStencilID stencilId)
{
	auto it = m_quadStencilComponents.find(stencilId);
	if (it != m_quadStencilComponents.end())
	{
		QuadStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_quadStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

BoxStencilComponentPtr StencilObjectSystem::getBoxStencilById(MikanStencilID stencilId) const
{
	auto iter = m_boxStencilComponents.find(stencilId);
	if (iter != m_boxStencilComponents.end())
	{
		return iter->second.lock();
	}

	return BoxStencilComponentPtr();
}

BoxStencilComponentPtr StencilObjectSystem::getBoxStencilByName(const std::string& stencilName) const
{
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		BoxStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return BoxStencilComponentPtr();
}

BoxStencilComponentPtr StencilObjectSystem::addNewBoxStencil(const MikanStencilBox& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewBoxStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		BoxStencilDefinitionPtr configPtr = stencilSystemConfig->getBoxStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createBoxStencilObject(configPtr);
	}

	return BoxStencilComponentPtr();
}

bool StencilObjectSystem::removeBoxStencil(MikanStencilID stencilId)
{
	return
		disposeBoxStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantBoxStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<BoxStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_boxStencilComponents.begin(); it != m_boxStencilComponents.end(); it++)
	{
		MikanSpatialAnchorID stencilId= it->first;
		BoxStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getStencilComponentDefinition()->getIsDisabled())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(
				allowedStencilIds->begin(), allowedStencilIds->end(), stencilId)
				== allowedStencilIds->end())
			{
				continue;
			}
		}

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

		{
			const glm::mat4 worldXform = componentPtr->getWorldTransform();
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilZAxis = glm::vec3(worldXform[2]); // Z is 2nd column
			const glm::vec3 stencilYAxis = glm::vec3(worldXform[1]); // Y is 1st column
			const glm::vec3 stencilXAxis = glm::vec3(worldXform[0]); // X is 0th column
			BoxStencilDefinitionConstPtr configPtr= componentPtr->getBoxStencilDefinition();
			const float boxXSize= configPtr->getBoxXSize();
			const float boxYSize= configPtr->getBoxYSize();
			const float boxZSize= configPtr->getBoxZSize();
			const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;

			const bool bIsStencilInFrontOfCamera = glm::dot(cameraToStencil, cameraForward) > 0.f;
			const bool bIsCameraInStecil =
				fabsf(glm::dot(cameraToStencil, stencilXAxis)) <= boxXSize &&
				fabsf(glm::dot(cameraToStencil, stencilYAxis)) <= boxYSize &&
				fabsf(glm::dot(cameraToStencil, stencilZAxis)) <= boxZSize;

			if (bIsStencilInFrontOfCamera || bIsCameraInStecil)
			{
				outStencilList.push_back(componentPtr);
			}
		}
	}
}

BoxStencilComponentPtr StencilObjectSystem::createBoxStencilObject(BoxStencilDefinitionPtr boxConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(boxConfig->getComponentName());

	// Make the box stencil the root scene component
	BoxStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<BoxStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	stencilComponentPtr->setDefinition(boxConfig);

	// Attach a box collider component to the stencil
	BoxColliderComponentPtr boxColliderPtr = stencilObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(
		glm::vec3(
			boxConfig->getBoxXSize() * 0.5f,
			boxConfig->getBoxYSize() * 0.5f,
			boxConfig->getBoxZSize() * 0.5f));
	boxColliderPtr->attachToComponent(stencilComponentPtr);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Init the object once all components are added
	stencilObject->init();

	// Keep track of all the box stencils in the stencil system
	m_boxStencilComponents.insert({boxConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeBoxStencilObject(MikanStencilID stencilId)
{
	auto it = m_boxStencilComponents.find(stencilId);
	if (it != m_boxStencilComponents.end())
	{
		BoxStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_boxStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

ModelStencilComponentPtr StencilObjectSystem::getModelStencilById(MikanStencilID stencilId) const
{
	auto iter = m_modelStencilComponents.find(stencilId);
	if (iter != m_modelStencilComponents.end())
	{
		return iter->second.lock();
	}

	return ModelStencilComponentPtr();
}

ModelStencilComponentPtr StencilObjectSystem::getModelStencilByName(const std::string& stencilName) const
{
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getDefinition()->getComponentName() == stencilName)
		{
			return componentPtr;
		}
	}

	return ModelStencilComponentPtr();
}

ModelStencilComponentPtr StencilObjectSystem::addNewModelStencil(const MikanStencilModel& stencilInfo)
{
	StencilObjectSystemConfigPtr stencilSystemConfig = getStencilSystemConfig();

	MikanStencilID stencilId = stencilSystemConfig->addNewModelStencil(stencilInfo);
	if (stencilId != INVALID_MIKAN_ID)
	{
		ModelStencilDefinitionPtr configPtr = stencilSystemConfig->getModelStencilConfig(stencilId);
		assert(configPtr != nullptr);

		return createModelStencilObject(configPtr);
	}

	return ModelStencilComponentPtr();
}

bool StencilObjectSystem::removeModelStencil(MikanStencilID stencilId)
{
	return 
		disposeModelStencilObject(stencilId) &&
		getStencilSystemConfig()->removeStencil(stencilId);
}

void StencilObjectSystem::getRelevantModelStencilList(
	const std::vector<MikanStencilID>* allowedStencilIds,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<ModelStencilComponentPtr>& outStencilList) const
{
	outStencilList.clear();
	for (auto it = m_modelStencilComponents.begin(); it != m_modelStencilComponents.end(); it++)
	{
		MikanStencilID stencilId = it->first;
		ModelStencilComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getModelStencilDefinition()->getIsDisabled())
			continue;

		if (componentPtr->getModelStencilDefinition()->getModelPath().empty())
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilIds != nullptr)
		{
			if (std::find(allowedStencilIds->begin(), allowedStencilIds->end(), stencilId) == allowedStencilIds->end())
			{
				continue;
			}
		}

		if (!isStencilFacingCamera(componentPtr, cameraPosition, cameraForward))
			continue;

		outStencilList.push_back(componentPtr);
	}
}

ModelStencilComponentPtr StencilObjectSystem::createModelStencilObject(ModelStencilDefinitionPtr modelConfig)
{
	MikanObjectPtr stencilObject = newObject();
	stencilObject->setName(modelConfig->getComponentName());

	// Make the model stencil component the root object
	ModelStencilComponentPtr stencilComponentPtr = stencilObject->addComponent<ModelStencilComponent>();
	stencilObject->setRootComponent(stencilComponentPtr);
	// Setting the config will spawn child mesh componets, if any
	stencilComponentPtr->setDefinition(modelConfig);

	// Add a selection component
	stencilObject->addComponent<SelectionComponent>();

	// Init the object once all components are added
	stencilObject->init();

	// Create mesh mesh components for the assigned mesh target
	stencilComponentPtr->rebuildMeshComponents();

	// Add the model stencil to the list of stencils
	m_modelStencilComponents.insert({modelConfig->getStencilId(), stencilComponentPtr});

	return stencilComponentPtr;
}

bool StencilObjectSystem::disposeModelStencilObject(MikanStencilID stencilId)
{
	auto it = m_modelStencilComponents.find(stencilId);
	if (it != m_modelStencilComponents.end())
	{
		ModelStencilComponentPtr stencilComponentPtr = it->second.lock();

		// Remove for component list
		m_modelStencilComponents.erase(it);

		// Free the corresponding object
		deleteObject(stencilComponentPtr->getOwnerObject());

		return true;
	}

	return false;
}

StencilObjectSystemConfigConstPtr StencilObjectSystem::getStencilSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->stencilConfig;
}

StencilObjectSystemConfigPtr StencilObjectSystem::getStencilSystemConfig()
{
	return std::const_pointer_cast<StencilObjectSystemConfig>(getStencilSystemConfigConst());
}

bool StencilObjectSystem::isStencilFacingCamera(
	StencilComponentConstPtr stencil,
	const glm::vec3& cameraPosition, const glm::vec3& cameraForward)
{
	StencilComponentConfigConstPtr configPtr= stencil->getStencilComponentDefinition();
	eStencilCullMode cullMode= configPtr->getCullMode();

	if (cullMode == eStencilCullMode::none)
		return true;

	glm::mat4 stencilXform= stencil->getWorldTransform();
	glm::vec3 stencilCenter= glm_mat4_get_position(stencilXform);
	glm::vec3 stencilForward;
	switch (cullMode)
	{
	case eStencilCullMode::zAxis:
		stencilForward= glm_mat4_get_z_axis(stencilXform);
		break;
	case eStencilCullMode::yAxis:
		stencilForward= glm_mat4_get_y_axis(stencilXform);
		break;
	case eStencilCullMode::xAxis:
		stencilForward= glm_mat4_get_x_axis(stencilXform);
		break;
	}

	const glm::vec3 cameraToStencil= stencilCenter - cameraPosition;
	const glm::vec3 stencilToCamera= -cameraToStencil;

	return 
		glm::dot(cameraToStencil, cameraForward) > 0.f &&
		glm::dot(stencilToCamera, stencilForward) > 0.f;
}

GlRenderModelResourcePtr StencilObjectSystem::loadStencilRenderModel(
	IGlWindow* ownerWindow,
	ModelStencilDefinitionPtr stencilDefinition)
{
	const GlVertexDefinition& vertexDefinition = getStencilModelVertexDefinition();
	GlModelResourceManager* modelResourceManager = ownerWindow->getModelResourceManager();
	GlTextureCache* textureCache= ownerWindow->getTextureCache();

	// Load the texture, if any, specified in the stencil definition
	const std::filesystem::path& texturePath = stencilDefinition->getTexturePath();
	GlTexturePtr texture = textureCache->loadTexturePath(texturePath);

	// Create the appropriate material for the stencil based on if a texture is present
	GlMaterialConstPtr stencilMaterial =
		(texture != nullptr) ? 
		createTexturedStencilMaterial(ownerWindow) :
		createVertexOnlyStencilMaterial(ownerWindow);
	
	// Attempt to load the render model
	GlRenderModelResourcePtr renderModelResource =
		modelResourceManager->fetchRenderModel(
			stencilDefinition->getModelPath(),
			&vertexDefinition,
			stencilMaterial);

	if (renderModelResource != nullptr)
	{
		// If we have a texture, apply it to the material instances of the render model
		if (texture)
		{
			for (int meshIndex = 0; meshIndex < renderModelResource->getTriangulatedMeshCount(); meshIndex++)
			{
				GlMaterialInstancePtr matInst = renderModelResource->getTriangulatedMeshMaterial(meshIndex);

				if (matInst)
				{
					matInst->setTextureBySemantic(eUniformSemantic::texture0, texture);
				}
			}
		}
	}
	else
	{
		// Free the texture if the model failed to load
		textureCache->removeTexureFromCache(texture);
	}

	return renderModelResource;
}

GlMaterialPtr StencilObjectSystem::createVertexOnlyStencilMaterial(IGlWindow* ownerWindow)
{
	GlProgramPtr program= 
		ownerWindow->getShaderCache()->fetchCompiledGlProgram(
			getVertexOnlyStencilShaderCode());

	return std::make_shared<GlMaterial>("Vertex Only Stencil Material", program);
}

GlMaterialPtr StencilObjectSystem::createTexturedStencilMaterial(IGlWindow* ownerWindow)
{
	GlProgramPtr program =
		ownerWindow->getShaderCache()->fetchCompiledGlProgram(
			getTexturedStencilShaderCode());

	return std::make_shared<GlMaterial>("Textured Stencil Material", program);
}

const GlVertexDefinition& StencilObjectSystem::getStencilModelVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const uint32_t positionSize = (uint32_t)sizeof(float) * 3;
		const uint32_t normalSize = (uint32_t)sizeof(float) * 3;
		const uint32_t texelSize = (uint32_t)sizeof(float) * 2;
		const uint32_t vertexSize = positionSize + normalSize + texelSize;
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		size_t offset = 0;
		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, offset));
		offset += positionSize;

		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::normal3f, false, vertexSize, offset));
		offset += normalSize;

		attribs.push_back(GlVertexAttribute(2, eVertexSemantic::texel2f, false, vertexSize, offset));
		offset += texelSize;

		assert(offset == vertexSize);
		x_vertexDefinition.vertexSize = vertexSize;
	}

	return x_vertexDefinition;
}

const GlProgramCode* StencilObjectSystem::getVertexOnlyStencilShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal Vertex Only Stencil Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec3 aPos;

			uniform mat4 mvpMatrix;

			void main()
			{
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			void main()
			{    
				FragColor = vec4(1, 1, 1, 1);
			}
			)"""")
		.addUniform(STENCIL_MVP_UNIFORM_NAME, eUniformSemantic::modelViewProjectionMatrix);

	return &x_shaderCode;
}

const GlProgramCode* StencilObjectSystem::getTexturedStencilShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal Textured Stencil Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 2) in vec3 aNormal;
			layout (location = 2) in vec2 aTexCoords;

			uniform mat4 mvpMatrix;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}  
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D rgbTexture;

			void main()
			{
				vec3 col = texture(rgbTexture, TexCoords).rgb;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
		.addUniform(STENCIL_MVP_UNIFORM_NAME, eUniformSemantic::modelViewProjectionMatrix)
		.addUniform("rgbTexture", eUniformSemantic::texture0);

	return &x_shaderCode;
}
