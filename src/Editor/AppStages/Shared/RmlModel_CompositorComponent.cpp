#include "Shared/RmlModel_CompositorComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "CameraObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CompositorComponent::RmlModel_CompositorComponent()
	: RmlModel_MikanComponent()
	, m_cameraIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_CompositorComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<CompositorComponent>(rmlContext);
}

bool RmlModel_CompositorComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all camera IDs from the CameraObjectSystem
	m_cameraIdList->init(
		constructor,
		CommonConfigPtr(),
		"camera_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			auto cameraObjectSystem= getCameraObjectSystem();
			if (cameraObjectSystem)
			{
				cameraObjectSystem->getTypedDefinition()->getAllComponentIds(outComponentIdList);
				outComponentIdList.insert(outComponentIdList.begin(), INVALID_MIKAN_ID); // Add "No Camera" option
			}
		});

	constructor.BindEventCallback(
		"select_camera_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int selectedCameraId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);

			CompositorComponentPtr compositorComponent= getCompositorComponent();
			if (compositorComponent)
				compositorComponent->getCompositorDefinition()->setCameraId(selectedCameraId);
		});

	constructor.BindEventCallback(
		"edit_compositor_graph",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			CompositorComponentPtr compositorComponent = getCompositorComponent();
			if (compositorComponent)
				compositorComponent->editCompositorGraph();
		});
	constructor.BindEventCallback(
		"add_new_compositor_graph",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			CompositorComponentPtr compositorComponent = getCompositorComponent();
			if (compositorComponent)
				compositorComponent->addNewCompositorGraph();
		});
	constructor.BindEventCallback(
		"remove_compositor_graph",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			CompositorComponentPtr compositorComponent = getCompositorComponent();
			if (compositorComponent)
				compositorComponent->removeCompositorGraph();
		});

	return true;
}

bool RmlModel_CompositorComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_cameraIdList->setOwnerConfig(getCameraObjectSystemConfig());
		m_cameraIdList->rebuildList(true);

		return true;
	}

	return false;
}

CameraObjectSystemPtr RmlModel_CompositorComponent::getCameraObjectSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<CameraObjectSystem>();
	}

	return nullptr;
}

CameraObjectSystemDefinitionPtr RmlModel_CompositorComponent::getCameraObjectSystemConfig() const
{
	auto cameraObjectSystem = getCameraObjectSystem();
	if (cameraObjectSystem)
	{
		return cameraObjectSystem->getTypedDefinition();
	}

	return nullptr;
}

CompositorComponentPtr RmlModel_CompositorComponent::getCompositorComponent() const
{
	return std::static_pointer_cast<CompositorComponent>(m_component.lock());
}