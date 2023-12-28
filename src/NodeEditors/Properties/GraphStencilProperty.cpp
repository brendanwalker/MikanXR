#include "GraphStencilProperty.h"
#include "GlMaterial.h"
#include "GlVertexDefinition.h"
#include "Logger.h"
#include "ProfileConfigConstants.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"

#include "Nodes/StencilNode.h"

#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"
#include "QuadStencilComponent.h"
#include "StencilObjectSystem.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- GraphStencilPropertyConfig -----
configuru::Config GraphStencilPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["stencil_type"]= k_stencilTypeStrings[(int)stencilType];
	pt["stencil_name"]= stencilName;

	return pt;
}

void GraphStencilPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string stencilTypeString =
		pt.get_or<std::string>(
			"stencil_type",
			k_patternTypeStrings[(int)eStencilType::quad]);
	stencilType =
		StringUtils::FindEnumValue<eStencilType>(
			stencilTypeString,
			k_stencilTypeStrings);
	stencilName = pt.get_or<std::string>("stencil_name", "");

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphStencilProperty -----
bool GraphStencilProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& stencilPropConfig = std::static_pointer_cast<const GraphStencilPropertyConfig>(propConfig);
		if (!stencilPropConfig->stencilName.empty() && stencilPropConfig->stencilType != eStencilType::INVALID)
		{
			auto stencilSystem= StencilObjectSystem::getSystem();

			switch (stencilPropConfig->stencilType)
			{
			case eStencilType::box:
				setStencilComponent(stencilSystem->getBoxStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::box;
				break;
			case eStencilType::model:
				setStencilComponent(stencilSystem->getModelStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::model;
				break;
			case eStencilType::quad:
				setStencilComponent(stencilSystem->getQuadStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::quad;
				break;
			default:
				MIKAN_LOG_ERROR("GraphStencilProperty::loadFromConfig") 
					<< "Invalid stencil name: " << stencilPropConfig->stencilName;
				setStencilComponent(StencilComponentPtr());
				m_stencilType= eStencilType::INVALID;
			}
		}
		else
		{
			// Config says the property had an empty stencil reference
			setStencilComponent(StencilComponentPtr());
			return true;
		}
	}

	return false;
}

void GraphStencilProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto stencilPropConfig = std::static_pointer_cast<GraphStencilPropertyConfig>(config);
	auto stencilSystem= StencilObjectSystem::getSystem();

	stencilPropConfig->stencilName= m_stencilComponent->getDefinition()->getComponentName();
	stencilPropConfig->stencilType= stencilSystem->getStencilType(stencilPropConfig->id);

	GraphProperty::saveToConfig(config);
}

void GraphStencilProperty::editorHandleDragDrop(const class NodeEditorState& editorState)
{
	auto stencilNode = m_ownerGraph->createTypedNode<StencilNode>(editorState);

	// Set this as the source model property for the new node
	auto self = std::static_pointer_cast<GraphStencilProperty>(shared_from_this());
	stencilNode->setStencilSource(self);
}

void GraphStencilProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	// Section 1: Basic info
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Stencil", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Name
		ImGui::Text("\t\tName");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		std::string name = m_stencilComponent ? m_stencilComponent->getName() : "<No Stencil>";
		ImGui::Text(name.c_str());

		// Stencil Type
		int stencilTypeIdex = (int)m_stencilType; 
		if (ImGui::Combo("Source Stencil Type", &stencilTypeIdex, k_szStencilTypeStrings, (int)eStencilType::COUNT))
		{
			setStencilComponent(StencilComponentPtr());
			m_stencilType= (eStencilType)stencilTypeIdex;
		}

		// Stencil
		{
			ImGui::Text("\t\tSource Stencil");
			ImGui::SameLine(160);
			ImGui::SetNextItemWidth(150);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

			// TODO: Need this overly complicated wrapper so that we can iterate over available
			// stencil names based on currently selected stencil type.
			// This should probably be a helper on the stencil object system instead.
			struct StencilComboDataSource
			{
				std::vector<StencilComponentPtr> stencils;
				int stencilSourceIndex = -1;

				StencilComboDataSource(
					StencilComponentPtr stencilComponent,
					eStencilType stencilType)
				{
					auto stencilSystem = StencilObjectSystem::getSystem();
					int listIndex = 0;

					switch (stencilType)
					{
						case eStencilType::box:
							for (auto it = stencilSystem->getBoxStencilMap().begin();
								 it != stencilSystem->getBoxStencilMap().end();
								 it++)
							{
								auto stencilPtr = it->second.lock();
								if (stencilPtr == stencilComponent)
								{
									stencilSourceIndex = listIndex;
								}
								stencils.push_back(stencilPtr);
								listIndex++;
							}
							break;
						case eStencilType::quad:
							for (auto it = stencilSystem->getQuadStencilMap().begin();
								 it != stencilSystem->getQuadStencilMap().end();
								 it++)
							{
								auto stencilPtr = it->second.lock();
								if (stencilPtr == stencilComponent)
								{
									stencilSourceIndex = listIndex;
								}
								stencils.push_back(stencilPtr);
								listIndex++;
							}
							break;
						case eStencilType::model:
							for (auto it = stencilSystem->getModelStencilMap().begin();
								 it != stencilSystem->getModelStencilMap().end();
								 it++)
							{
								auto stencilPtr = it->second.lock();
								if (stencilPtr == stencilComponent)
								{
									stencilSourceIndex = listIndex;
								}
								stencils.push_back(stencilPtr);
								listIndex++;
							}
							break;
						default:
							break;
					}
				}

				static bool ItemGetter(void* data, int idx, const char** out_str)
				{
					auto* dataSource = (StencilComboDataSource*)data;
					StencilComponentPtr stencil = dataSource->stencils[idx];
					const char* stencilName = stencil->getName().c_str();

					*out_str = stencilName;
					return true;
				}
			};
			StencilComboDataSource dataSource(m_stencilComponent, m_stencilType);
			if (ImGui::Combo("##stencilSelection",
							 &dataSource.stencilSourceIndex,
							 &StencilComboDataSource::ItemGetter, &dataSource, (int)dataSource.stencils.size()))
			{
				setStencilComponent(dataSource.stencils[dataSource.stencilSourceIndex]);
			}

			ImGui::PopStyleColor();
		}
	}
}