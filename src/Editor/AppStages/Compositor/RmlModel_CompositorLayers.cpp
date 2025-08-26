#include "AssetReference.h"
#include "RmlModel_CompositorLayers.h"
#include "CompositorComponent.h"
#include "StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Event.h>

#include <vector>

bool RmlModel_CompositorLayers::init(
	Rml::Context* rmlContext,
	CompositorComponentPtr compositor)
{
	m_compositor= compositor;
	m_compositorDefinition = compositor->getCompositorDefinition();

	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_layers");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("compositor_graph_path", &m_compositorGraphPath);

	// Bind data model callbacks
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

	// Listen for profile config changes
	m_compositorDefinition->OnMarkedDirty += MakeDelegate(this, &RmlModel_CompositorLayers::onCompositorConfigMarkedDirty);

	return true;
}

void RmlModel_CompositorLayers::dispose()
{
	m_compositorDefinition->OnMarkedDirty -= MakeDelegate(this, &RmlModel_CompositorLayers::onCompositorConfigMarkedDirty);
	m_compositorDefinition = nullptr;

	m_compositor = nullptr;

	OnGraphEditEvent.Clear();
	OnGraphFileSelectEvent.Clear();
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
	if (changedPropertySet.hasPropertyName(CompositorDefinition::k_compositorGraphPathPropertyId))
	{
		setCompositorGraphPath(m_compositorDefinition->getCompositorGraphPath().c_str());
	}
}