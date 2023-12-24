#include "GraphMaterialProperty.h"
#include "GlMaterial.h"
#include "GlVertexDefinition.h"
#include "Graphs/NodeGraph.h"
#include "Logger.h"
#include "MaterialAssetReference.h"
#include "Nodes/MaterialNode.h"

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

void GraphMaterialProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	auto materialNode =
		std::static_pointer_cast<MaterialNode>(
			MaterialNodeFactory().createNode(editorState));

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
		std::string name = m_materialResource->getName();
		ImGui::Text(name.c_str());

		// TODO show material properties
	}
}