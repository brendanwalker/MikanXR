#include "AppStage.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"

bool GuiPanel_SpoutTextureSourceComponent::init()
{
	m_spoutTextureSourceSystem= getOwnerAppStage()->getObjectSystemOfType<SpoutTextureSourceSystem>();
	return initTypedPropertyInterface<SpoutTextureSourceComponent>();
}

void GuiPanel_SpoutTextureSourceComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		SpoutTextureSourceDefinition::k_spoutSourcePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto textureSourceComp= getSpoutTextureSourceComponent();
			if (!textureSourceComp)
				return false;

			m_spoutSenderDataSource.setEntries(m_spoutSenderNames);

			const std::string& currentSource= textureSourceComp->getSpoutTextureSourceDefinition()->getSpoutSource();
			int selectedIndex= m_spoutSenderDataSource.getEntryIndexByString(currentSource);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					textureSourceComp->makePropertyUIIdentifier(SpoutTextureSourceDefinition::k_spoutSourcePropertyId),
					"Spout Source", &m_spoutSenderDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newSource= m_spoutSenderDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent(
						[textureSourceComp, newSource]()
						{ textureSourceComp->getSpoutTextureSourceDefinition()->setSpoutSource(newSource); });
				}
			}
			return true;
		});
}

void GuiPanel_SpoutTextureSourceComponent::update(float deltaSeconds)
{
	// Periodically refresh the spout sender names list
	SpoutTextureSourceSystemPtr spoutSystem= getSpoutTextureSourceSystem();
	if (spoutSystem)
	{
		m_timeSinceLastSourceListRefresh+= deltaSeconds;
		if (m_timeSinceLastSourceListRefresh >= k_spoutSourceListUpdateInterval)
		{
			m_timeSinceLastSourceListRefresh= 0.0f;

			m_spoutSenderNames.clear();
			spoutSystem->getAvailableSpoutSenderNames(m_spoutSenderNames);
		}
	}
}

SpoutTextureSourceComponentPtr GuiPanel_SpoutTextureSourceComponent::getSpoutTextureSourceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<SpoutTextureSourceComponent>(component);
	}
	return nullptr;
}

SpoutTextureSourceSystemPtr GuiPanel_SpoutTextureSourceComponent::getSpoutTextureSourceSystem() const
{
	return m_spoutTextureSourceSystem.lock();
}
