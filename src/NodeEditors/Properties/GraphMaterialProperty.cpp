#include "GraphMaterialProperty.h"
#include "GlMaterial.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "MaterialAssetReference.h"
#include "NodeEditorUI.h"
#include "Nodes/MaterialNode.h"
#include "Properties/GraphVariableList.h"
#include "IGlWindow.h"
#include "GlShaderCache.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- MaterialAssetComboDataSource ---
class MaterialAssetComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	MaterialAssetComboDataSource(GraphMaterialPropertyPtr ownerProperty)
	{
		auto ownerGraph = ownerProperty->getOwnerGraph();
		int listIndex = 0;

		currentAssetRef = ownerProperty->getMaterialAssetReference();

		for (AssetReferencePtr assetRef : ownerGraph->getAssetReferences())
		{
			auto matAssetRef = std::dynamic_pointer_cast<MaterialAssetReference>(assetRef);

			if (matAssetRef)
			{
				if (matAssetRef == currentAssetRef)
				{
					selectedAssetRefIndex = listIndex;
				}

				ComboEntry entry = {
					matAssetRef,
					matAssetRef ? assetRef->getShortName() : "<No Asset Ref>"
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

	inline MaterialAssetReferencePtr getEntryAssetRef(int index)
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
		MaterialAssetReferencePtr assetReference;
		std::string entryString;
	};

	MaterialAssetReferencePtr currentAssetRef;
	std::vector<ComboEntry> comboEntries;
	int selectedAssetRefIndex = -1;
};

// -- GraphMaterialPropertyConfig -----
configuru::Config GraphMaterialPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["asset_ref_index"]= assetRefIndex;

	return pt;
}

void GraphMaterialPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	assetRefIndex = pt.get_or<int>("asset_ref_index", -1);

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphMaterialProperty -----
bool GraphMaterialProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& matPropConfig = std::static_pointer_cast<const GraphMaterialPropertyConfig>(propConfig);
		if (matPropConfig->assetRefIndex != -1)
		{
			auto assetRef = getOwnerGraph()->getAssetReferenceByIndex(matPropConfig->assetRefIndex);
			auto materialAssetRef= std::dynamic_pointer_cast<MaterialAssetReference>(assetRef);
			if (materialAssetRef)
			{
				setMaterialAssetReference(materialAssetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("GraphMaterialProperty::loadFromConfig") 
					<< "Invalid material asset reference: " << matPropConfig->assetRefIndex;
				setMaterialAssetReference(MaterialAssetReferencePtr());
			}
		}
		else
		{
			// Config says the property had an empty asset reference
			setMaterialAssetReference(MaterialAssetReferencePtr());
			return true;
		}
	}

	return false;
}

void GraphMaterialProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto propConfig = std::static_pointer_cast<GraphMaterialPropertyConfig>(config);

	// Default asset ref to invalid
	propConfig->assetRefIndex = -1;

	// If we have a valid asset ref, look up the index in the graph
	if (m_materialAssetRef)
	{
		propConfig->assetRefIndex = getOwnerGraph()->getAssetReferenceIndex(m_materialAssetRef);

		if (propConfig->assetRefIndex == -1)
		{
			MIKAN_LOG_ERROR("GraphMaterialProperty::saveToConfig") << "Material property has orphaned asset reference: " << m_materialAssetRef->getAssetPath();
		}
	}

	GraphProperty::saveToConfig(config);
}

void GraphMaterialProperty::setMaterialAssetReference(MaterialAssetReferencePtr inAssetRef) 
{ 
	if (m_materialAssetRef != inAssetRef)
	{
		m_materialAssetRef= inAssetRef;

		// re-create a material from the asset reference
		if (m_materialAssetRef->isValid())
		{
			GlShaderCache* shaderCache = getOwnerGraph()->getOwnerWindow()->getShaderCache();
			assert(shaderCache);

			m_materialResource = shaderCache->loadMaterialAssetReference(m_materialAssetRef);
		}
		else
		{
			m_materialResource= GlMaterialPtr();
		}
	}
}

void GraphMaterialProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	auto materialNode = m_ownerGraph->createTypedNode<MaterialNode>(editorState);

	// Set this as the source model property for the new node
	auto self = std::static_pointer_cast<GraphMaterialProperty>(shared_from_this());
	materialNode->setMaterialSource(self);
}

void GraphMaterialProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Material"))
	{
		// Name
		std::string name = m_materialResource ? m_materialResource->getName() : "";
		NodeEditorUI::DrawStaticTextProperty("Name", name);

		// Material Asset
		MaterialAssetComboDataSource dataSource(std::static_pointer_cast<GraphMaterialProperty>(shared_from_this()));
		int selectedIndex= dataSource.getCurrentAssetIndex();
		if (NodeEditorUI::DrawComboBoxProperty("Material Asset", "materialSelection", &dataSource, selectedIndex))
		{
			setMaterialAssetReference(dataSource.getEntryAssetRef(selectedIndex));
		}
	}
}