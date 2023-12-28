#include "GraphMaterialProperty.h"
#include "GlMaterial.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "MaterialAssetReference.h"
#include "Nodes/MaterialNode.h"
#include "Properties/GraphVariableList.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

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

		// TODO Create a material from the asset reference
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
	// Section 1: Basic info
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Name
		ImGui::Text("\t\tName");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		std::string name = m_materialResource ? m_materialResource->getName() : "";
		ImGui::Text(name.c_str());

		// Material Asset
		{
			ImGui::Text("\t\tMaterial Asset");
			ImGui::SameLine(160);
			ImGui::SetNextItemWidth(150);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

			struct MaterialAssetComboDataSource
			{
				struct ComboEntry
				{
					MaterialAssetReferencePtr assetReference;
					std::string entryString;
				};
				MaterialAssetReferencePtr currentAssetRef;
				std::vector<ComboEntry> comboEntries;
				int selectedAssetRefIndex = -1;

				MaterialAssetComboDataSource(GraphMaterialPropertyPtr ownerProperty)
				{
					auto ownerGraph = ownerProperty->getOwnerGraph();
					int listIndex = 0;

					currentAssetRef= ownerProperty->getMaterialAssetReference();

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

				static bool ItemGetter(void* data, int idx, const char** out_str)
				{
					auto* dataSource = (MaterialAssetComboDataSource*)data;

					*out_str = dataSource->comboEntries[idx].entryString.c_str();
					return true;
				}
			};
			MaterialAssetComboDataSource dataSource(std::static_pointer_cast<GraphMaterialProperty>(shared_from_this()));
			if (ImGui::Combo("##stencilSelection",
							 &dataSource.selectedAssetRefIndex,
							 &MaterialAssetComboDataSource::ItemGetter, 
							 &dataSource, (int)dataSource.comboEntries.size()))
			{
				setMaterialAssetReference(dataSource.comboEntries[dataSource.selectedAssetRefIndex].assetReference);
			}

			ImGui::PopStyleColor();
		}
	}
}