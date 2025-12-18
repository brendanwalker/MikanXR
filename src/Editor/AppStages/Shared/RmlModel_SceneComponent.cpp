#include "RmlModel_SceneComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_PropertyInterface.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_SceneComponent::RmlModel_SceneComponent()
	: RmlModel_MikanComponent()
	, m_compositorIdList(std::make_shared<RmlDataBinding_ComponentIdList>())
{}

bool RmlModel_SceneComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<SceneComponent>(rmlContext);
}

bool RmlModel_SceneComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	// Build the list of all compositor IDs this scene component can output to
	m_compositorIdList->init(
		constructor,
		CommonConfigPtr(),
		"compositor_ids",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<int>& outComponentIdList) {
			SceneComponentPtr sceneComponent= getSceneComponent();
			if (sceneComponent)
			{
				outComponentIdList = sceneComponent->getOutputCompositorIDs();
			}
		});

	constructor.BindEventCallback(
		"select_compositor_entry",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const int newCompositorId = ev.GetParameter<int>("value", INVALID_MIKAN_ID);
			SceneComponentPtr sceneComponent = getSceneComponent();
			if (sceneComponent)
			{
				sceneComponent->getSceneComponentDefinition()->setDisplayCompositorId(newCompositorId);
			}
		});

	return true;
}

bool RmlModel_SceneComponent::setComponent(MikanComponentPtr component)
{
	if (RmlModel_MikanComponent::setComponent(component))
	{
		m_compositorIdList->setOwnerConfig(getSceneComponent()->getSceneComponentDefinition());
		m_compositorIdList->rebuildList(true);

		return true;
	}

	return false;
}

SceneComponentPtr RmlModel_SceneComponent::getSceneComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<SceneComponent>(component);
	}

	return nullptr;
}