#include "AnchorObjectSystem.h"
#include "Colors.h"
#include "GlLineRenderer.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlTriangulatedMesh.h"
#include "GlTextRenderer.h"
#include "GlStaticMeshInstance.h"
#include "GlWireframeMesh.h"
#include "AnchorComponent.h"
#include "Renderer.h"
#include "SceneComponent.h"
#include "SelectionComponent.h"
#include "StaticMeshComponent.h"
#include "StencilObjectSystem.h"
#include "StencilObjectSystemConfig.h"
#include "MathGLM.h"
#include "MathMikan.h"
#include "MathTypeConversion.h"
#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "ModelStencilComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
const std::string ModelStencilDefinition::k_modelStencilObjPathPropertyId = "model_path";

ModelStencilDefinition::ModelStencilDefinition()
	: StencilComponentDefinition()
{
}

ModelStencilDefinition::ModelStencilDefinition(const MikanStencilModel& modelInfo)
	: StencilComponentDefinition(
		modelInfo.stencil_id, 
		modelInfo.parent_anchor_id, 
		modelInfo.stencil_name, 
		modelInfo.relative_transform)
{
}

configuru::Config ModelStencilDefinition::writeToJSON()
{
	configuru::Config pt = StencilComponentDefinition::writeToJSON();

	pt["model_path"] = m_modelPath.string();

	return pt;
}

void ModelStencilDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_modelPath = pt.get<std::string>("model_path");
}

void ModelStencilDefinition::setModelPath(const std::filesystem::path& path)
{
	m_modelPath= path;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilObjPathPropertyId));
}

// -- ModelStencilComponent -----
ModelStencilComponent::ModelStencilComponent(MikanObjectWeakPtr owner)
	: StencilComponent(owner)
{
	m_bWantsCustomRender= true;
}

void ModelStencilComponent::init()
{
	StencilComponent::init();

	// Create a selection component so that we can selection the mesh collision geometry
	SelectionComponentPtr selectionComponentPtr = getOwnerObject()->getComponentOfType<SelectionComponent>();
	if (selectionComponentPtr)
	{
		// Bind selection events
		selectionComponentPtr->OnInteractionRayOverlapEnter += MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit += MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected += MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected += MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);

		// Remember the selection component
		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
}

void ModelStencilComponent::customRender()
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();

	if (!modelStencilDefinition->getIsDisabled())
	{
		TextStyle style = getDefaultTextStyle();

		const glm::mat4 xform = getWorldTransform();
		const glm::vec3 position = glm::vec3(xform[3]);

		drawTransformedAxes(xform, 0.1f, 0.1f, 0.1f);
		drawTextAtWorldPosition(style, position, L"Stencil %d", modelStencilDefinition->getStencilId());
	}
}

void ModelStencilComponent::dispose()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->OnInteractionRayOverlapEnter -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapEnter);
		selectionComponentPtr->OnInteractionRayOverlapExit -= MakeDelegate(this, &ModelStencilComponent::onInteractionRayOverlapExit);
		selectionComponentPtr->OnInteractionSelected -= MakeDelegate(this, &ModelStencilComponent::onInteractionSelected);
		selectionComponentPtr->OnInteractionUnselected -= MakeDelegate(this, &ModelStencilComponent::onInteractionUnselected);

		m_selectionComponentWeakPtr = selectionComponentPtr;
	}

	StencilComponent::dispose();
}

void ModelStencilComponent::setModelPath(const std::filesystem::path& path)
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();

	if (path == modelStencilDefinition->getModelPath())
		return;

	modelStencilDefinition->setModelPath(path);
	rebuildMeshComponents();
}

void ModelStencilComponent::rebuildMeshComponents()
{
	ModelStencilDefinitionPtr modelStencilDefinition= getModelStencilDefinition();
	MikanObjectPtr stencilObject= getOwnerObject();
	StencilComponentPtr stencilComponentPtr= getSelfPtr<StencilComponent>();

	// Clean up any previously created mesh components
	while (m_meshComponents.size() > 0)
	{
		SceneComponentPtr componentPtr= m_meshComponents[m_meshComponents.size() - 1];
		componentPtr->dispose();

		m_meshComponents.pop_back();
	}

	// Forget about any wireframe meshes
	m_wireframeMeshes.clear();

	// Fetch the model resource
	GlRenderModelResourcePtr modelResourcePtr;
	if (!modelStencilDefinition->getModelPath().empty())
	{
		auto& modelResourceManager = Renderer::getInstance()->getModelResourceManager();
		modelResourcePtr = modelResourceManager->fetchRenderModel(
			modelStencilDefinition->getModelPath(),
			GlRenderModelResource::getDefaultVertexDefinition());
	}

	// If a model loaded, create meshes and colliders for it
	if (modelResourcePtr)
	{
		// Add static tri meshes
		for (size_t meshIndex = 0; meshIndex < modelResourcePtr->getTriangulatedMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			GlTriangulatedMeshPtr triMeshPtr = modelResourcePtr->getTriangulatedMesh((int)meshIndex);
			GlMaterialInstancePtr materialInstancePtr = modelResourcePtr->getTriangulatedMeshMaterial((int)meshIndex);

			// Create a new static mesh instance from the mesh resources
			GlStaticMeshInstancePtr triMeshInstancePtr =
				std::make_shared<GlStaticMeshInstance>(
					triMeshPtr->getName(),
					triMeshPtr,
					materialInstancePtr);
			triMeshInstancePtr->setVisible(false);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
			meshComponentPtr->setName(triMeshPtr->getName());
			meshComponentPtr->setStaticMesh(triMeshInstancePtr);
			meshComponentPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(meshComponentPtr);

			// Add a mesh collider component that generates collision from the mesh data
			MeshColliderComponentPtr colliderPtr = stencilObject->addComponent<MeshColliderComponent>();
			colliderPtr->setName(triMeshPtr->getName());
			colliderPtr->setStaticMeshComponent(meshComponentPtr);
			colliderPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(colliderPtr);
		}

		// Add static wireframe meshes
		for (size_t meshIndex = 0; meshIndex < modelResourcePtr->getWireframeMeshCount(); ++meshIndex)
		{
			// Fetch the mesh and material resources
			GlWireframeMeshPtr wireframeMeshPtr = modelResourcePtr->getWireframeMesh((int)meshIndex);
			GlMaterialInstancePtr materialInstancePtr = modelResourcePtr->getWireframeMeshMaterial((int)meshIndex);

			// Create a new (hidden) static mesh instance from the mesh resources
			GlStaticMeshInstancePtr wireframeMeshInstancePtr =
				std::make_shared<GlStaticMeshInstance>(
					"wireframe",
					wireframeMeshPtr,
					materialInstancePtr);
			wireframeMeshInstancePtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA,
				glm::vec4(Colors::DarkGray, 1.f));
			m_wireframeMeshes.push_back(wireframeMeshInstancePtr);

			// Create a static mesh component to hold the mesh instance
			StaticMeshComponentPtr meshComponentPtr = stencilObject->addComponent<StaticMeshComponent>();
			meshComponentPtr->setName(wireframeMeshPtr->getName());
			meshComponentPtr->setStaticMesh(wireframeMeshInstancePtr);
			meshComponentPtr->attachToComponent(stencilComponentPtr);
			m_meshComponents.push_back(meshComponentPtr);
		}

		// Initialize all of the newly created components
		for (SceneComponentPtr childComponentPtr : m_meshComponents)
		{
			childComponentPtr->init();
		}
	}

	// Refresh the child collider list on the selection component
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		selectionComponentPtr->rebindColliders();
	}
}

void ModelStencilComponent::onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult)
{
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr && !selectionComponentPtr->getIsSelected())
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA, 
				glm::vec4(Colors::LightGray, 1.f));
			//meshPtr->setVisible(true);
		}
	}
}

void ModelStencilComponent::onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult)
{
	SelectionComponentPtr selectionComponentPtr= m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr && !selectionComponentPtr->getIsSelected())
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA,
				glm::vec4(Colors::DarkGray, 1.f));
			//meshPtr->setVisible(false);
		}
	}
}

void ModelStencilComponent::onInteractionSelected()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
		{
			meshPtr->getMaterialInstance()->setVec4BySemantic(
				eUniformSemantic::diffuseColorRGBA, 
				glm::vec4(Colors::Yellow, 1.f));
			//meshPtr->setVisible(true);
		}
	}
}

void ModelStencilComponent::onInteractionUnselected()
{
	SelectionComponentPtr selectionComponentPtr = m_selectionComponentWeakPtr.lock();
	if (selectionComponentPtr)
	{
		if (selectionComponentPtr->getIsHovered())
		{
			for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
			{
				meshPtr->getMaterialInstance()->setVec4BySemantic(
					eUniformSemantic::diffuseColorRGBA, 
					glm::vec4(Colors::LightGray, 1.f));
				//meshPtr->setVisible(true);
			}
		}
		else
		{
			for (GlStaticMeshInstancePtr meshPtr : m_wireframeMeshes)
			{
				meshPtr->getMaterialInstance()->setVec4BySemantic(
					eUniformSemantic::diffuseColorRGBA,
					glm::vec4(Colors::DarkGray, 1.f));
				//meshPtr->setVisible(false);
			}
		}
	}
}

// -- IPropertyInterface ----
void ModelStencilComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	StencilComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(ModelStencilDefinition::k_modelStencilObjPathPropertyId);
}

bool ModelStencilComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (StencilComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	else if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		outDescriptor = {ModelStencilDefinition::k_modelStencilObjPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}

	return false;
}

bool ModelStencilComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (StencilComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		Rml::String filepath= getModelStencilDefinition()->getModelPath().string();

		outValue= filepath;
		return true;
	}

	return false;
}

bool ModelStencilComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (StencilComponent::getPropertyAttribute(propertyName, attributeName, outValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue= "Select a model";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = "*.obj";
		}
	}

	return false;
}

bool ModelStencilComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (StencilComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == ModelStencilDefinition::k_modelStencilObjPathPropertyId)
	{
		const Rml::String fileString= inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		setModelPath(filePath);
		return true;
	}

	return false;
}