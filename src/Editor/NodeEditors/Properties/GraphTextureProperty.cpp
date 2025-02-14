#include "GraphTextureProperty.h"
#include "IMkTexture.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "NodeEditorUI.h"
#include "Nodes/TextureNode.h"
#include "TextureAssetReference.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- TextureAssetComboDataSource ---
class TextureAssetComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	TextureAssetComboDataSource(GraphTexturePropertyPtr ownerProperty)
	{
		auto ownerGraph = ownerProperty->getOwnerGraph();
		int listIndex = 0;

		currentAssetRef = ownerProperty->getTextureAssetReference();

		for (AssetReferencePtr assetRef : ownerGraph->getAssetReferences())
		{
			auto textureAssetRef = std::dynamic_pointer_cast<TextureAssetReference>(assetRef);

			if (textureAssetRef)
			{
				if (textureAssetRef == currentAssetRef)
				{
					selectedAssetRefIndex = listIndex;
				}

				ComboEntry entry = {
					textureAssetRef,
					textureAssetRef ? assetRef->getShortName() : "<No Asset Ref>"
				};

				comboEntries.push_back(entry);
				listIndex++;
			}
		}
	}

	inline int getCurrentAssetIndex() const
	{
		return selectedAssetRefIndex;
	}

	inline TextureAssetReferencePtr getEntryAssetRef(int index)
	{
		return comboEntries[index].assetReference;
	}

	virtual int getEntryCount() override
	{
		return (int)comboEntries.size();
	}

	virtual const std::string& getEntryDisplayString(int index) override
	{
		return comboEntries[index].entryString;
	}

private:
	struct ComboEntry
	{
		TextureAssetReferencePtr assetReference;
		std::string entryString;
	};

	TextureAssetReferencePtr currentAssetRef;
	std::vector<ComboEntry> comboEntries;
	int selectedAssetRefIndex = -1;
};

// -- GraphTexturePropertyConfig -----
configuru::Config GraphTexturePropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["asset_ref_index"] = assetRefIndex;

	return pt;
}

void GraphTexturePropertyConfig::readFromJSON(const configuru::Config& pt)
{
	assetRefIndex = pt.get_or<int>("asset_ref_index", -1);

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphModelProperty -----
bool GraphTextureProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto texturePropConfig = std::static_pointer_cast<const GraphTexturePropertyConfig>(propConfig);
		if (texturePropConfig->assetRefIndex != -1)
		{
			auto assetRef = getOwnerGraph()->getAssetReferenceByIndex(texturePropConfig->assetRefIndex);
			auto textureAssetRef = std::dynamic_pointer_cast<TextureAssetReference>(assetRef);
			if (textureAssetRef)
			{
				setTextureAssetReference(textureAssetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("GraphTextureProperty::loadFromConfig") 
					<< "Invalid texture asset reference: " << texturePropConfig->assetRefIndex;
				setTextureAssetReference(TextureAssetReferencePtr());
			}
		}
		else
		{
			// Config says the property had an empty asset reference
			setTextureAssetReference(TextureAssetReferencePtr());
			return true;
		}
	}

	return false;
}

void GraphTextureProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto propConfig = std::static_pointer_cast<GraphTexturePropertyConfig>(config);

	// Default asset ref to invalid
	propConfig->assetRefIndex = -1;

	// If we have a valid asset ref, look up the index in the graph
	if (m_textureAssetRef)
	{
		propConfig->assetRefIndex = getOwnerGraph()->getAssetReferenceIndex(m_textureAssetRef);

		if (propConfig->assetRefIndex == -1)
		{
			MIKAN_LOG_ERROR("GraphTextureProperty::saveToConfig") 
				<< "Texture property has orphaned asset reference: " 
				<< m_textureAssetRef->getAssetPath();
		}
	}

	GraphProperty::saveToConfig(config);
}

void GraphTextureProperty::setTextureAssetReference(TextureAssetReferencePtr inAssetRef) 
{ 
	if (m_textureAssetRef != inAssetRef)
	{
		m_textureAssetRef = inAssetRef;

		// re-create a texture from the asset reference
		if (m_textureAssetRef->isValid())
		{
			m_texture = CreateMkTexture();
			m_texture->setImagePath(inAssetRef->getAssetPath());
			m_texture->reloadTextureFromImagePath();
		}
		else
		{
			m_texture = IMkTexturePtr();
		}
	}
}

void GraphTextureProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto textureNode = m_ownerGraph->createTypedNode<TextureNode>(editorState);

	// Set this as the source texture property for the new node
	auto self= std::static_pointer_cast<GraphTextureProperty>(shared_from_this());
	textureNode->setTextureSource(self);
}

void GraphTextureProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Texture"))
	{
		// Name
		std::string name = m_textureAssetRef ? m_textureAssetRef->getShortName() : "<No Texture>";
		NodeEditorUI::DrawStaticTextProperty("Name", name);

		// Texture Asset
		TextureAssetComboDataSource dataSource(std::static_pointer_cast<GraphTextureProperty>(shared_from_this()));
		int selectedIndex = dataSource.getCurrentAssetIndex();
		if (NodeEditorUI::DrawComboBoxProperty("textureSelection", "Texture", &dataSource, selectedIndex))
		{
			setTextureAssetReference(dataSource.getEntryAssetRef(selectedIndex));
		}

		// Drag-Drop Handling
		auto textureAssetRef =
			NodeEditorUI::receiveTypedDragDropPayload<TextureAssetReference>(
				TextureAssetReference::k_assetClassName);
		if (textureAssetRef)
		{
			setTextureAssetReference(textureAssetRef);
		}

		// Texture
		IMkTexturePtr texture= getTextureResource();
		NodeEditorUI::DrawImageProperty("Preview", texture);
	}
}

const ImVec4 GraphTextureProperty::editorGetIconColor() const
{
	return NodeEditorUI::getTextureColor();
}