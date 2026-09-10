#include "MaterialNodeGraph.h"
#include "Logger.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"

// Asset References
#include "TextureAssetReference.h"

// Pins
#include "Pins/ShaderValuePin.h"

// Pages
#include "Graphs/MaterialFunctionPage.h"

// Nodes
#include "Nodes/Material/MaterialNodeLibrary.h"
#include "Nodes/Material/MaterialOutputNode.h"
#include "Nodes/Material/ShaderFunctionNodes.h"

// Compiler
#include "MaterialCompiler/MaterialCompiler.h"

namespace
{
const char* k_domainSettingKey= "domain";
const char* k_vertexPresetSettingKey= "vertex_preset";
} // namespace

// -- MaterialNodeGraph -----
MaterialNodeGraph::MaterialNodeGraph()
	: NodeGraph()
{
	// Texture parameters name a default texture for the preview
	addAssetReferenceFactory<TextureAssetReferenceFactory>();

	addPinFactory<ShaderValuePin, ShaderValuePinConfig>();

	addNodeFactory<MaterialOutputNodeFactory>();
	MaterialNodeLibrary::registerNodeFactories(*this);

	// Material functions: a page per function, its input and output nodes, and one
	// call factory per page registered as pages come and go
	addPageFactory<MaterialFunctionPageFactory>();
	addNodeFactory<ShaderFunctionInputNodeFactory>();
	addNodeFactory<ShaderFunctionOutputNodeFactory>();
	addNodeFactory<ShaderFunctionCallNodeFactory>(-1, std::string());
	OnPageCreated+= MakeDelegate(this, &MaterialNodeGraph::onFunctionPageCreated);
	OnPageModified+= MakeDelegate(this, &MaterialNodeGraph::onFunctionPageModified);
	OnPageDeleted+= MakeDelegate(this, &MaterialNodeGraph::onFunctionPageDeleted);
}

bool MaterialNodeGraph::loadPageFromConfig(GraphPageConfigPtr pageConfig)
{
	if (!NodeGraph::loadPageFromConfig(pageConfig))
		return false;

	registerFunctionCallFactory(pageConfig->id);

	return true;
}

void MaterialNodeGraph::registerFunctionCallFactory(t_graph_page_id pageId)
{
	auto page= std::dynamic_pointer_cast<MaterialFunctionPage>(getPageById(pageId));
	if (!page)
		return;

	// Re-registering replaces the entry, which is how a rename reaches the create menu
	removeNodeFactory(ShaderFunctionCallNodeFactory::makeFactoryKey(pageId));
	addNodeFactory<ShaderFunctionCallNodeFactory>(pageId, page->getName());
}

void MaterialNodeGraph::onFunctionPageCreated(t_graph_page_id pageId) { registerFunctionCallFactory(pageId); }

void MaterialNodeGraph::onFunctionPageModified(t_graph_page_id pageId) { registerFunctionCallFactory(pageId); }

void MaterialNodeGraph::onFunctionPageDeleted(t_graph_page_id pageId)
{
	removeNodeFactory(ShaderFunctionCallNodeFactory::makeFactoryKey(pageId));
}

bool MaterialNodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	// Domain first, so the output node's pins load against the right domain
	const std::string domainName=
		config.settings.get_or<std::string>(k_domainSettingKey, MaterialDomainUtils::domainToString(m_domain));
	const eMaterialDomain domain= MaterialDomainUtils::domainFromString(domainName);
	m_domain= (domain != eMaterialDomain::INVALID) ? domain : eMaterialDomain::compositor;

	const std::string presetName= config.settings.get_or<std::string>(
		k_vertexPresetSettingKey, MaterialDomainUtils::presetToString(MaterialDomainUtils::getDefaultPreset(m_domain)));
	const eMaterialVertexPreset preset= MaterialDomainUtils::presetFromString(presetName);
	m_vertexPreset= MaterialDomainUtils::isPresetAllowed(m_domain, preset)
						? preset
						: MaterialDomainUtils::getDefaultPreset(m_domain);

	if (!NodeGraph::loadFromConfig(config))
	{
		MIKAN_LOG_ERROR("MaterialNodeGraph::loadFromConfig") << "Failed to parse node graph config";
		return false;
	}

	if (!getOutputNode())
	{
		MIKAN_LOG_ERROR("MaterialNodeGraph::loadFromConfig") << "Material graph has no output node";
		return false;
	}

	return true;
}

void MaterialNodeGraph::saveToConfig(NodeGraphConfig& config) const
{
	NodeGraph::saveToConfig(config);

	config.settings[k_domainSettingKey]= MaterialDomainUtils::domainToString(m_domain);
	config.settings[k_vertexPresetSettingKey]= MaterialDomainUtils::presetToString(m_vertexPreset);
}

void MaterialNodeGraph::setDomain(eMaterialDomain domain)
{
	if (domain == eMaterialDomain::INVALID || domain == m_domain)
		return;

	m_domain= domain;
	m_vertexPreset= MaterialDomainUtils::getDefaultPreset(domain);

	MaterialOutputNodePtr outputNode= getOutputNode();
	if (outputNode)
	{
		outputNode->rebuildPinsForDomain(domain);
	}

	if (OnDomainChanged)
	{
		OnDomainChanged();
	}
}

void MaterialNodeGraph::setVertexPreset(eMaterialVertexPreset preset)
{
	if (preset == m_vertexPreset || !MaterialDomainUtils::isPresetAllowed(m_domain, preset))
		return;

	m_vertexPreset= preset;

	if (OnDomainChanged)
	{
		OnDomainChanged();
	}
}

MaterialOutputNodePtr MaterialNodeGraph::getOutputNode() const
{
	NodePtr node= getNodeByPredicate([](const auto& entry)
									 { return entry.second->getClassName() == MaterialOutputNode::k_nodeClassName; });

	return std::static_pointer_cast<MaterialOutputNode>(node);
}

MaterialCompileResult MaterialNodeGraph::compile(const IShaderWriter& writer)
{
	MaterialCompileResult result=
		MaterialCompiler::compile(std::static_pointer_cast<MaterialNodeGraph>(shared_from_this()), writer);
	m_lastCompileErrors= result.errors;

	return result;
}

void MaterialNodeGraph::editorRenderGraphPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("materialEditor.graphHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		// Domain
		std::string domainItems;
		for (int index= 0; index < (int)eMaterialDomain::COUNT; ++index)
		{
			domainItems+= MaterialDomainUtils::domainToString((eMaterialDomain)index);
			domainItems+= '\0';
		}
		int iDomain= (int)m_domain;
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "materialGraphDomain", locText("materialEditor.domain"),
											  domainItems.c_str(), iDomain))
		{
			setDomain((eMaterialDomain)iDomain);
		}

		// Vertex preset, limited to what the domain allows
		const std::vector<eMaterialVertexPreset>& presets= MaterialDomainUtils::getAllowedPresets(m_domain);
		std::string presetItems;
		int iPreset= 0;
		for (size_t index= 0; index < presets.size(); ++index)
		{
			presetItems+= MaterialDomainUtils::presetToString(presets[index]);
			presetItems+= '\0';
			if (presets[index] == m_vertexPreset)
			{
				iPreset= (int)index;
			}
		}
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "materialGraphVertexPreset",
											  locText("materialEditor.vertexPreset"), presetItems.c_str(), iPreset))
		{
			if (iPreset >= 0 && iPreset < (int)presets.size())
			{
				setVertexPreset(presets[iPreset]);
			}
		}
	}
}

// -- MaterialNodeGraphFactory -----
NodeGraphPtr MaterialNodeGraphFactory::initialCreateNodeGraph(IEditorWindow* ownerWindow) const
{
	return initialCreateMaterialGraph(ownerWindow, eMaterialDomain::compositor);
}

NodeGraphPtr MaterialNodeGraphFactory::initialCreateMaterialGraph(IEditorWindow* ownerWindow,
																  eMaterialDomain domain) const
{
	auto nodeGraph= NodeGraphFactory::initialCreateNodeGraph(ownerWindow);
	if (!nodeGraph)
		return NodeGraphPtr();

	auto materialGraph= std::static_pointer_cast<MaterialNodeGraph>(nodeGraph);
	materialGraph->m_domain= (domain != eMaterialDomain::INVALID) ? domain : eMaterialDomain::compositor;
	materialGraph->m_vertexPreset= MaterialDomainUtils::getDefaultPreset(materialGraph->m_domain);

	// Every material graph carries exactly one output node
	NodeEditorState editorState;
	editorState.nodeGraph= materialGraph;
	materialGraph->createTypedNode<MaterialOutputNode>(editorState);

	return materialGraph;
}
