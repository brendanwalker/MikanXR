#include "RmlModel_NetworkVideoSourceComponent.h"
#include "Shared/RmlDataBinding_List.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "NetworkVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSettings/RmlDataBinding_VideoSourceSetting.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

// -- RmlModel_NetworkVideoSourceComponent -----
bool RmlModel_NetworkVideoSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<NetworkVideoSourceComponent>(rmlContext);
}

bool RmlModel_NetworkVideoSourceComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	constructor.BindEventCallback(
		"select_protocol",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newProtocol = ev.GetParameter<Rml::String>("value", "");
			auto videoSourceComponent = getNetworkVideoSourceComponent();
			if (videoSourceComponent)
			{
				videoSourceComponent->getNetworkVideoSourceDefinition()->setProtocol(newProtocol);
			}
		});

	return true;
}

NetworkVideoSourceComponentPtr RmlModel_NetworkVideoSourceComponent::getNetworkVideoSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<NetworkVideoSourceComponent>(component);
	}

	return nullptr;
}