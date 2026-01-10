#include "RmlModel_SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "Shared/RmlDataBinding_List.h"

RmlModel_SpoutTextureSourceComponent::RmlModel_SpoutTextureSourceComponent()
	: m_spoutSourceList(std::make_shared<RmlDataBinding_SpoutSourceList>())
{
}

bool RmlModel_SpoutTextureSourceComponent::init(Rml::Context* rmlContext)
{
	return initTypedPropertyInterface<SpoutTextureSourceComponent>(rmlContext);
}

bool RmlModel_SpoutTextureSourceComponent::onConstruct(Rml::DataModelConstructor& constructor)
{
	if (!RmlModel_MikanComponent::onConstruct(constructor))
		return false;

	constructor.BindEventCallback(
		"select_spout_source",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			const auto newSourceName = ev.GetParameter<Rml::String>("value", "");
			auto textureSourceComponent = getSpoutTextureSourceComponent();
			if (textureSourceComponent)
			{
				textureSourceComponent->getSpoutTextureSourceDefinition()->setSpoutSource(newSourceName);
			}
		});

	// Build the list of all available spout sources
	m_spoutSourceList->init(
		constructor,
		CommonConfigPtr(),
		"spout_sender_names",
		[this](CommonConfigPtr ownerConfig, Rml::Vector<Rml::String>& outSenderNames)
		{
			auto spoutTextureSourceSystem = getSpoutTextureSourceSystem();
			if (spoutTextureSourceSystem)
			{
				spoutTextureSourceSystem->getAvailableSpoutSenderNames(outSenderNames);
			}
		});

	return true;
}

void RmlModel_SpoutTextureSourceComponent::update(float deltaSeconds)
{
	RmlModel_MikanComponent::update(deltaSeconds);

	SpoutTextureSourceSystemPtr spoutTextureSourceSystem = getSpoutTextureSourceSystem();
	if (spoutTextureSourceSystem)
	{
		m_timeSinceLastSourceListRefresh += deltaSeconds;
		if (m_timeSinceLastSourceListRefresh >= k_spoutSourceListUpdateInterval)
		{
			m_timeSinceLastSourceListRefresh = 0.0f;
			m_spoutSourceList->rebuildList();
		}
	}
}

SpoutTextureSourceComponentPtr RmlModel_SpoutTextureSourceComponent::getSpoutTextureSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<SpoutTextureSourceComponent>(component);
	}

	return nullptr;
}

SpoutTextureSourceSystemPtr RmlModel_SpoutTextureSourceComponent::getSpoutTextureSourceSystem() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return component->getObjectSystemOfType<SpoutTextureSourceSystem>();
	}

	return nullptr;
}
