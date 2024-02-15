#include "AssetReference.h"
#include "RmlModel_CompositorLayers.h"
#include "GlFrameCompositor.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Event.h>

#include <vector>

bool RmlModel_CompositorLayers::init(
	Rml::Context* rmlContext,
	GlFrameCompositor* compositor)
{
	m_compositor= compositor;
	m_compositorConfig= compositor->getConfigMutable();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_layers");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("current_configuration", &m_currentConfigurationName);
	constructor.Bind("is_builtin_configuration", &m_bIsBuiltInConfiguration);
	constructor.Bind("compositor_graph_path", &m_compositorGraphPath);
	constructor.Bind("configuration_names", &m_configurationNames);

	// Bind data model callbacks
	constructor.BindEventCallback(
		"add_config",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnConfigAddEvent) OnConfigAddEvent();
		});
	constructor.BindEventCallback(
		"delete_config",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnConfigDeleteEvent) OnConfigDeleteEvent();
		});
	constructor.BindEventCallback(
		"modify_config_name",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnConfigNameChangeEvent && ev.GetId() == Rml::EventId::Change)
			{
				const bool isLineBreak = ev.GetParameter("linebreak", false);
				const std::string newConfigName = ev.GetParameter<Rml::String>("value", "");
				if (isLineBreak && !newConfigName.empty())
				{
						OnConfigNameChangeEvent(newConfigName);
				}
			}
		});

	constructor.BindEventCallback(
		"edit_compositor_graph",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnGraphEditEvent) OnGraphEditEvent();
		});
	constructor.BindEventCallback(
		"select_compositor_graph_file",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnGraphFileSelectEvent) OnGraphFileSelectEvent();
		});

	constructor.BindEventCallback(
		"select_configuration",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			if (OnConfigSelectEvent) 
			{
				const std::string configurationName = ev.GetParameter<Rml::String>("value", "");
				OnConfigSelectEvent(configurationName);
			}
		});

	// Listen for profile config changes
	m_compositorConfig->OnMarkedDirty += MakeDelegate(this, &RmlModel_CompositorLayers::onCompositorConfigMarkedDirty);

	// Set initial values for data model
	onCurrentPresetChanged();

	return true;
}

void RmlModel_CompositorLayers::dispose()
{
	if (m_presetConfig)
	{
		m_presetConfig->OnMarkedDirty -= MakeDelegate(this, &RmlModel_CompositorLayers::onPresetConfigMarkedDirty);
		m_presetConfig = nullptr;
	}

	m_compositorConfig->OnMarkedDirty -= MakeDelegate(this, &RmlModel_CompositorLayers::onCompositorConfigMarkedDirty);
	m_compositorConfig = nullptr;

	OnConfigSelectEvent.Clear();
	RmlModel::dispose();
}

const std::filesystem::path RmlModel_CompositorLayers::getCompositorGraphPath() const
{
	return m_compositorGraphPath;
}

void RmlModel_CompositorLayers::setCompositorGraphPath(const std::filesystem::path& path)
{
	m_compositorGraphPath= path.string();
	m_modelHandle.DirtyVariable("compositor_graph_path");
}

void RmlModel_CompositorLayers::onCompositorConfigMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(GlFrameCompositorConfig::k_presetNamePropertyId))
	{
		onCurrentPresetChanged();
	}
}

void RmlModel_CompositorLayers::onCurrentPresetChanged()
{
	CompositorPresetPtr newPreset= m_compositor->getCurrentPresetConfigMutable();

	if (m_presetConfig)
	{
		m_presetConfig->OnMarkedDirty -= MakeDelegate(this, &RmlModel_CompositorLayers::onPresetConfigMarkedDirty);
		m_presetConfig = nullptr;
	}

	if (newPreset)
	{
		newPreset->OnMarkedDirty += MakeDelegate(this, &RmlModel_CompositorLayers::onPresetConfigMarkedDirty);
		m_presetConfig= newPreset;
	}

	// Rebuild the data model
	rebuild(m_compositor);
}

void RmlModel_CompositorLayers::onPresetConfigMarkedDirty(
	CommonConfigPtr configPtr, 
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(CompositorPreset::k_compositorGraphAssetRefPropertyId))
	{
		setCompositorGraphPath(m_presetConfig->compositorGraphAssetRefConfig->assetPath);
	}
}

void RmlModel_CompositorLayers::rebuild(
	const GlFrameCompositor* compositor)
{
	assert(m_presetConfig);

	m_configurationNames = compositor->getPresetNames();
	m_currentConfigurationName= m_presetConfig->name;
	m_bIsBuiltInConfiguration= m_presetConfig->builtIn;

	// Apply the selected compositor graph path
	{
		AssetReferenceConfigPtr assetRefConfigPtr = m_presetConfig->compositorGraphAssetRefConfig;
		const std::string assetRefPath = assetRefConfigPtr ? assetRefConfigPtr->assetPath : "";

		setCompositorGraphPath(assetRefPath);
	}

	m_modelHandle.DirtyVariable("is_builtin_configuration");
	m_modelHandle.DirtyVariable("configuration_names");
	m_modelHandle.DirtyVariable("current_configuration");
}