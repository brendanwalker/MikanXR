#include "App.h"
#include "AssetReferencePropertyMetaData.h"
#include "FunctionInterface.h"
#include "IMkGraphicsContext.h"
#include "IMkTexture.h"
#include "Logger.h"
#include "MikanVariantTypes.h"
#include "MikanObject.h"
#include "NodeGraphAssetReference.h"
#include "ProjectManager.h"
#include "PropertyInterface.h"
#include "ShapeComponent.h"
#include "Windows/ShapeNodeEditorWindow.h"
#include "TextureSourceComponent.h"
#include "TextureSourceQueries.h"
#include "VideoDisplayConstants.h"

#include "Graphs/NodeEvaluator.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/ShapeNodeGraph.h"

#include "tinyfiledialogs.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- ShapeComponentDefinition ------
const std::string ShapeComponentDefinition::k_textureSourceIdPropertyId = "shape_texture_source_id";
const std::string ShapeComponentDefinition::k_shapeGraphPathPropertyId = "shape_graph_path";

ShapeComponentDefinition::ShapeComponentDefinition()
	: TransformComponentDefinition()
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{
}

ShapeComponentDefinition::ShapeComponentDefinition(MikanShapeID shapeId)
	: TransformComponentDefinition(shapeId)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{
}

configuru::Config ShapeComponentDefinition::writeToJSON()
{
	configuru::Config pt = TransformComponentDefinition::writeToJSON();

	pt[k_textureSourceIdPropertyId] = m_textureSourceId;

	if (m_nodeGraphAssetRef)
	{
		pt[k_shapeGraphPathPropertyId] = m_nodeGraphAssetRef->writeToJSON();
	}

	return pt;
}

void ShapeComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	TransformComponentDefinition::readFromJSON(pt);

	m_textureSourceId = pt.get_or<int>(k_textureSourceIdPropertyId, INVALID_MIKAN_ID);

	m_nodeGraphAssetRef = NodeGraphAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_shapeGraphPathPropertyId))
	{
		m_nodeGraphAssetRef->readFromJSON(pt[k_shapeGraphPathPropertyId]);
	}
}

void ShapeComponentDefinition::setTextureSourceId(MikanTextureSourceID id)
{
	if (m_textureSourceId != id)
	{
		m_textureSourceId = id;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_textureSourceIdPropertyId));
	}
}

bool ShapeComponentDefinition::hasShapeGraphPath() const
{
	return m_nodeGraphAssetRef && !m_nodeGraphAssetRef->assetPath.empty();
}

std::filesystem::path ShapeComponentDefinition::getShapeGraphPath() const
{
	return m_nodeGraphAssetRef ? m_nodeGraphAssetRef->assetPath : std::filesystem::path();
}

void ShapeComponentDefinition::setShapeGraphPath(const std::filesystem::path& graphPath)
{
	if (!m_nodeGraphAssetRef || graphPath != m_nodeGraphAssetRef->assetPath)
	{
		if (!m_nodeGraphAssetRef)
			m_nodeGraphAssetRef = std::make_shared<AssetReferenceConfig>();
		m_nodeGraphAssetRef->assetPath = graphPath.string();
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_shapeGraphPathPropertyId));
	}
}

// -- ShapeComponent ------
const std::string ShapeComponent::k_addNewShapeGraphFunctionId = "add_new_shape_graph";
const std::string ShapeComponent::k_editShapeGraphFunctionId = "edit_shape_graph";
const std::string ShapeComponent::k_removeShapeGraphFunctionId = "remove_shape_graph";
const std::string ShapeComponent::k_selectShapeGraphFunctionId = "select_shape_graph";

ShapeComponent::ShapeComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsUpdate = true;
}

void ShapeComponent::init()
{
	TransformComponent::init();

	m_nodeGraphAssetRef =
		std::static_pointer_cast<NodeGraphAssetReference>(
			NodeGraphAssetReferenceFactory().allocateAssetReference());

	// Listen for changes to the shape definition
	getShapeComponentDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &ShapeComponent::onDefinitionChanged);
}

void ShapeComponent::postInit()
{
	TransformComponent::postInit();

	// Initialize the shape graph if we have one assigned
	handleShapeNodeGraphChanged(getShapeComponentDefinition()->getShapeGraphPath());
}

void ShapeComponent::dispose()
{
	getShapeComponentDefinition()->OnPropertyChanged -=
		MakeDelegate(this, &ShapeComponent::onDefinitionChanged);

	m_nodeGraph = nullptr;
	m_nodeGraphAssetRef = nullptr;

	TransformComponent::dispose();
}

void ShapeComponent::onDefinitionChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(ShapeComponentDefinition::k_shapeGraphPathPropertyId))
	{
		handleShapeNodeGraphChanged(getShapeComponentDefinition()->getShapeGraphPath());
	}
}

MikanTextureSourceID ShapeComponent::getTextureSourceId() const
{
	return getShapeComponentDefinition()->getTextureSourceId();
}

void ShapeComponent::setTextureSourceId(MikanTextureSourceID id)
{
	getShapeComponentDefinition()->setTextureSourceId(id);
}

void ShapeComponent::update(float deltaSeconds)
{
	// Refresh the cached texture from the referenced texture source component
	const MikanTextureSourceID textureSourceId = getTextureSourceId();
	if (textureSourceId != INVALID_MIKAN_ID)
	{
		ProjectManagerPtr projectManager = getOwnerProjectManager();
		TextureSourceComponentPtr textureSource =
			TextureSourceQueries::getTextureSourceById(projectManager, textureSourceId);

		if (textureSource)
		{
			m_colorTexture = textureSource->getClientColorSourceTexture(
				INVALID_MIKAN_ID,
				eTextureSourceColorType::colorRGBA,
				-1);
		}
		else
		{
			m_colorTexture = nullptr;
		}
	}
	else
	{
		m_colorTexture = nullptr;
	}
}

void ShapeComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	TransformComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	// Subclasses override this to respond to property changes (e.g. size change → rebuild mesh)
}

// -- Shape Node Graph ----
bool ShapeComponent::hasValidShapeGraph() const
{
	return m_nodeGraph != nullptr;
}

void ShapeComponent::setEditorShapeNodeGraph(ShapeNodeGraphPtr editorNodeGraph)
{
	m_editorNodeGraph = editorNodeGraph;
}

void ShapeComponent::addNewShapeGraph()
{
	removeShapeGraph();
	editShapeGraph();
}

void ShapeComponent::editShapeGraph()
{
	App* app = App::getInstance();

	if (!app->hasWindowOfType<ShapeNodeEditorWindow>())
	{
		app->createAppWindow<ShapeNodeEditorWindow>()
			->bindShapeComponent(getSelfPtr<ShapeComponent>());
	}
}

void ShapeComponent::removeShapeGraph()
{
	setShapeGraphAssetPath(std::filesystem::path());
}

void ShapeComponent::selectShapeGraph()
{
	NodeGraphAssetReferenceFactory assetRefFactory;
	const char* picked =
		tinyfd_openFileDialog(
			assetRefFactory.getFileDialogTitle(),
			assetRefFactory.getDefaultPath(),
			assetRefFactory.getFilterPatternCount(),
			assetRefFactory.getFilterPatterns(),
			assetRefFactory.getFilterDescription(),
			1);

	if (picked != nullptr && picked[0] != '\0')
	{
		setShapeGraphAssetPath(std::filesystem::path(picked));
	}
}

void ShapeComponent::renderShapeGraph(const glm::mat4& vpMatrix, IMkGraphicsContext* graphicsContext)
{
	// Editor graph takes priority over asset-based graph
	ShapeNodeGraphPtr nodeGraph = m_editorNodeGraph.lock();
	if (!nodeGraph)
		nodeGraph = m_nodeGraph;
	if (!nodeGraph)
		return;

	NodeEvaluator evaluator;
	evaluator.setCurrentGraphicsContext(graphicsContext);

	if (nodeGraph->renderShape(vpMatrix, evaluator))
	{
		m_lastNodeEvalErrors.clear();
	}
	else
	{
		m_lastNodeEvalErrors = evaluator.getErrors();
	}
}

std::filesystem::path ShapeComponent::getShapeGraphAssetPath() const
{
	return m_nodeGraphAssetRef ? m_nodeGraphAssetRef->getAssetPath() : std::filesystem::path();
}

void ShapeComponent::setShapeGraphAssetPath(const std::filesystem::path& assetRefPath)
{
	handleShapeNodeGraphChanged(assetRefPath);
	getShapeComponentDefinition()->setShapeGraphPath(assetRefPath);
}

void ShapeComponent::handleShapeNodeGraphChanged(const std::filesystem::path& newAssetRefPath)
{
	if (!m_nodeGraphAssetRef)
	{
		m_nodeGraphAssetRef =
			std::static_pointer_cast<NodeGraphAssetReference>(
				NodeGraphAssetReferenceFactory().allocateAssetReference());
	}

	m_nodeGraphAssetRef->setAssetPath(newAssetRefPath);

	if (m_nodeGraphAssetRef->isValid())
	{
		m_nodeGraph =
			std::dynamic_pointer_cast<ShapeNodeGraph>(
				NodeGraphFactory::loadNodeGraph(getOwnerEditorWindow(), newAssetRefPath));

		if (m_nodeGraph)
		{
			MIKAN_LOG_INFO("ShapeComponent::handleShapeNodeGraphChanged")
				<< "Loaded shape graph: " << newAssetRefPath;

			m_nodeGraph->bindToShapeComponent(getSelfPtr<ShapeComponent>());
		}
		else
		{
			MIKAN_LOG_ERROR("ShapeComponent::handleShapeNodeGraphChanged")
				<< "Failed to load shape graph: " << newAssetRefPath;
		}
	}
	else
	{
		m_nodeGraph = nullptr;
	}

	m_lastNodeEvalErrors.clear();
}

// -- IPropertyInterface ----
void ShapeComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			ShapeComponentDefinition::k_textureSourceIdPropertyId, MikanVariantType::INT)
		->setDefaultValue(INVALID_MIKAN_ID));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			ShapeComponentDefinition::k_shapeGraphPathPropertyId, MikanVariantType::STRING)
		->addMetaData(std::make_shared<AssetReferenceFactoryMetaData>(
			AssetReferenceFactory::createFactory<NodeGraphAssetReferenceFactory>())));
}

bool ShapeComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == ShapeComponentDefinition::k_textureSourceIdPropertyId)
	{
		outValue = (int)getTextureSourceId();
		return true;
	}
	else if (propertyName == ShapeComponentDefinition::k_shapeGraphPathPropertyId)
	{
		outValue = getShapeComponentDefinition()->getShapeGraphPath().string();
		return true;
	}

	return TransformComponent::getPropertyValue(propertyName, outValue);
}

bool ShapeComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == ShapeComponentDefinition::k_textureSourceIdPropertyId)
	{
		setTextureSourceId((MikanTextureSourceID)inValue.getIntValue());
		return true;
	}

	return TransformComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
void ShapeComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TransformComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_addNewShapeGraphFunctionId, "Add Shape Graph")
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_editShapeGraphFunctionId, "Edit Shape Graph")
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_removeShapeGraphFunctionId, "Remove Shape Graph")
		->setUIHidden());
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_selectShapeGraphFunctionId, "Select Shape Graph")
		->setUIHidden());
}

bool ShapeComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == k_addNewShapeGraphFunctionId)
	{
		addNewShapeGraph();
		return true;
	}
	else if (functionName == k_editShapeGraphFunctionId)
	{
		editShapeGraph();
		return true;
	}
	else if (functionName == k_removeShapeGraphFunctionId)
	{
		removeShapeGraph();
		return true;
	}
	else if (functionName == k_selectShapeGraphFunctionId)
	{
		selectShapeGraph();
		return true;
	}

	return TransformComponent::invokeFunction(functionName);
}

// -- Lua Binding ----
void ShapeComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<ShapeComponent, TransformComponent>("ShapeComponent")
		.addProperty("textureSourceId",
			[](ShapeComponent* component) -> int {
				return static_cast<int>(component->getTextureSourceId());
			},
			[](ShapeComponent* component, int id) {
				component->setTextureSourceId(static_cast<MikanTextureSourceID>(id));
			})
		.endClass();
}
