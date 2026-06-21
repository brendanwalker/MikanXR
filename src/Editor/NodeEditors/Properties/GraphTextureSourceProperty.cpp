#include "GraphTextureSourceProperty.h"
#include "MkMaterial.h"
#include "IMkVertexDefinition.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "ProjectConfigConstants.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"

#include "Nodes/TextureSourceNode.h"

#include "CEFTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "CEFTextureSourceSystem.h"
#include "ClientTextureSourceSystem.h"
#include "SpoutTextureSourceSystem.h"
#include "TextureSourceQueries.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- MaterialAssetComboDataSource ---
class TextureSourceComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	TextureSourceComboDataSource(NodeGraphPtr ownerGraph, TextureSourceComponentPtr textureSourceComponent,
								 eTextureSourceType textureSourceType)
	{
		if (!ownerGraph)
		{
			return;
		}

		int listIndex= 0;

		switch (textureSourceType)
		{
		case eTextureSourceType::cef:
		{
			auto cefTextureSourceSystem= ownerGraph->getObjectSystemOfType<CEFTextureSourceSystem>();
			for (auto it= cefTextureSourceSystem->getComponentMap().begin();
				 it != cefTextureSourceSystem->getComponentMap().end(); it++)
			{
				auto textureSourcePtr= it->second.lock();
				if (textureSourcePtr == textureSourceComponent)
				{
					textureSourceSourceIndex= listIndex;
				}
				comboEntries.push_back({textureSourcePtr, textureSourcePtr->getName()});
				listIndex++;
			}
		}
		break;
		case eTextureSourceType::client:
		{
			auto clientTextureSourceSystem= ownerGraph->getObjectSystemOfType<ClientTextureSourceSystem>();
			for (auto it= clientTextureSourceSystem->getComponentMap().begin();
				 it != clientTextureSourceSystem->getComponentMap().end(); it++)
			{
				auto textureSourcePtr= it->second.lock();
				if (textureSourcePtr == textureSourceComponent)
				{
					textureSourceSourceIndex= listIndex;
				}
				comboEntries.push_back({textureSourcePtr, textureSourcePtr->getName()});
				listIndex++;
			}
		}
		break;
		case eTextureSourceType::spout:
		{
			auto modelTextureSourceSystem= ownerGraph->getObjectSystemOfType<SpoutTextureSourceSystem>();
			for (auto it= modelTextureSourceSystem->getComponentMap().begin();
				 it != modelTextureSourceSystem->getComponentMap().end(); it++)
			{
				auto textureSourcePtr= it->second.lock();
				if (textureSourcePtr == textureSourceComponent)
				{
					textureSourceSourceIndex= listIndex;
				}
				comboEntries.push_back({textureSourcePtr, textureSourcePtr->getName()});
				listIndex++;
			}
		}
		break;
		default:
			break;
		}
	}

	inline int getCurrentTextureSourceIndex() const { return textureSourceSourceIndex; }

	inline TextureSourceComponentPtr getEntryTextureSource(int index)
	{
		assert(index >= 0 && index < (int)comboEntries.size());

		return comboEntries[index].TextureSource;
	}

	virtual int getEntryCount() override { return (int)comboEntries.size(); }

	virtual const std::string& getEntryDisplayString(int index) override
	{
		assert(index >= 0 && index < (int)comboEntries.size());
		return comboEntries[index].entryString;
	}

private:
	struct ComboEntry
	{
		TextureSourceComponentPtr TextureSource;
		std::string entryString;
	};

	std::vector<ComboEntry> comboEntries;
	int textureSourceSourceIndex= -1;
};

// -- GraphTextureSourcePropertyConfig -----
configuru::Config GraphTextureSourcePropertyConfig::writeToJSON()
{
	configuru::Config pt= GraphPropertyConfig::writeToJSON();

	pt["texture_source_type"]= (textureSourceType != eTextureSourceType::INVALID)
								   ? k_textureSourceTypeStrings[(int)textureSourceType]
								   : k_textureSourceTypeStrings[(int)eTextureSourceType::spout];
	pt["texture_source_name"]= textureSourceName;

	return pt;
}

void GraphTextureSourcePropertyConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string textureSourceTypeString=
		pt.get_or<std::string>("texture_source_type", k_textureSourceTypeStrings[(int)eTextureSourceType::spout]);
	textureSourceType=
		StringUtils::FindEnumValue<eTextureSourceType>(textureSourceTypeString, k_textureSourceTypeStrings);
	textureSourceName= pt.get_or<std::string>("texture_source_name", "");

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphTextureSourceProperty -----
bool GraphTextureSourceProperty::loadFromConfig(GraphPropertyConfigConstPtr propConfig,
												const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& textureSourcePropConfig=
			std::static_pointer_cast<const GraphTextureSourcePropertyConfig>(propConfig);
		if (!textureSourcePropConfig->textureSourceName.empty()
			&& textureSourcePropConfig->textureSourceType != eTextureSourceType::INVALID)
		{
			switch (textureSourcePropConfig->textureSourceType)
			{
			case eTextureSourceType::cef:
			{
				auto cefTextureSourceSystem= getOwnerGraph()->getObjectSystemOfType<CEFTextureSourceSystem>();
				setTextureSourceComponent(
					cefTextureSourceSystem->getTypedComponentByName(textureSourcePropConfig->textureSourceName));
				m_textureSourceType= eTextureSourceType::cef;
			}
			break;
			case eTextureSourceType::client:
			{
				auto clientTextureSourceSystem= getOwnerGraph()->getObjectSystemOfType<ClientTextureSourceSystem>();
				setTextureSourceComponent(
					clientTextureSourceSystem->getTypedComponentByName(textureSourcePropConfig->textureSourceName));
				m_textureSourceType= eTextureSourceType::client;
			}
			break;
			case eTextureSourceType::spout:
			{
				auto spoutTextureSourceSystem= getOwnerGraph()->getObjectSystemOfType<SpoutTextureSourceSystem>();
				setTextureSourceComponent(
					spoutTextureSourceSystem->getTypedComponentByName(textureSourcePropConfig->textureSourceName));
				m_textureSourceType= eTextureSourceType::spout;
			}
			break;
			default:
				MIKAN_LOG_ERROR("GraphTextureSourceProperty::loadFromConfig")
					<< "Invalid TextureSource name: " << textureSourcePropConfig->textureSourceName;
				setTextureSourceComponent(TextureSourceComponentPtr());
				m_textureSourceType= eTextureSourceType::INVALID;
			}

			return m_textureSourceType != eTextureSourceType::INVALID;
		}
		else
		{
			// Config says the property had an empty TextureSource reference
			setTextureSourceComponent(TextureSourceComponentPtr());
			return true;
		}
	}

	return false;
}

void GraphTextureSourceProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto TextureSourcePropConfig= std::static_pointer_cast<GraphTextureSourcePropertyConfig>(config);

	if (m_textureSourceComponent != nullptr)
	{
		TextureSourceDefinitionPtr definition= m_textureSourceComponent->getTextureSourceDefinition();

		TextureSourcePropConfig->textureSourceName= definition->getComponentName();
		TextureSourcePropConfig->id= definition->getComponentId();
		TextureSourcePropConfig->textureSourceType= TextureSourceQueries::getTextureSourceType(
			getOwnerGraph()->getOwnerProject(), definition->getComponentId());
	}
	else
	{
		TextureSourcePropConfig->textureSourceName= "";
		TextureSourcePropConfig->id= -1;
		TextureSourcePropConfig->textureSourceType= eTextureSourceType::INVALID;
	}

	GraphProperty::saveToConfig(config);
}

void GraphTextureSourceProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto textureSourceNode= m_ownerGraph->createTypedNode<TextureSourceNode>(editorState);

	// Set this as the source model property for the new node
	auto self= std::static_pointer_cast<GraphTextureSourceProperty>(shared_from_this());
	textureSourceNode->setTextureSourceProperty(self);
}

void GraphTextureSourceProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("TextureSource", editorState.styleManager))
	{
		// Name
		std::string name= m_textureSourceComponent ? m_textureSourceComponent->getName() : "<No TextureSource>";
		NodeEditorUI::DrawStaticTextProperty("Name", name, editorState.styleManager);

		// TextureSource Type
		int TextureSourceTypeIdex= (int)m_textureSourceType;
		if (ImGui::Combo("Type", &TextureSourceTypeIdex, k_szTextureSourceTypeStrings, (int)eTextureSourceType::COUNT))
		{
			setTextureSourceComponent(TextureSourceComponentPtr());
			m_textureSourceType= (eTextureSourceType)TextureSourceTypeIdex;
		}

		// TextureSource
		TextureSourceComboDataSource dataSource(m_ownerGraph, m_textureSourceComponent, m_textureSourceType);
		int selectedIndex= dataSource.getCurrentTextureSourceIndex();
		if (NodeEditorUI::DrawComboBoxProperty("TextureSourceSelection", "Source", &dataSource, selectedIndex,
											   editorState.styleManager))
		{
			setTextureSourceComponent(dataSource.getEntryTextureSource(selectedIndex));
		}
	}
}

const ImVec4 GraphTextureSourceProperty::editorGetIconColor() const { return NodeEditorUI::getComponentColor(); }