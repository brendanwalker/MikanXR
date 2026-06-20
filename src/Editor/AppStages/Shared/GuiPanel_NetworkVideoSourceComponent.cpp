#include "AppStage.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceComponent.h"

bool GuiPanel_NetworkVideoSourceComponent::init()
{
	return initTypedPropertyInterface<NetworkVideoSourceComponent>();
}

void GuiPanel_NetworkVideoSourceComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// Build static protocol string list once
	std::vector<std::string> protocolStrings;
	for (int i= 0; i < (int)eNetworkVideoProtocol::COUNT; ++i)
		protocolStrings.push_back(k_NetworkVideoProtocol[i]);
	m_protocolDataSource.setEntries(protocolStrings);

	m_entityAccessor->setPropertyRenderer(
		NetworkVideoSourceDefinition::k_protocolPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto videoSourceComp= getNetworkVideoSourceComponent();
			if (!videoSourceComp)
				return false;

			auto definitionPtr= videoSourceComp->getNetworkVideoSourceDefinition();
			const eNetworkVideoProtocol currentProtocol= definitionPtr->getProtocol();
			const int currentIndex=
				(currentProtocol != eNetworkVideoProtocol::INVALID) ? (int)currentProtocol : -1;
			int selectedIndex= currentIndex;

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					videoSourceComp->makePropertyUIIdentifier(NetworkVideoSourceDefinition::k_protocolPropertyId),
					"Protocol",
					&m_protocolDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const eNetworkVideoProtocol newProtocol= (eNetworkVideoProtocol)selectedIndex;
					addDeferredGuiEvent([videoSourceComp, newProtocol]()
										{ videoSourceComp->getNetworkVideoSourceDefinition()->setProtocol(newProtocol); });
				}
			}
			return true;
		});
}

NetworkVideoSourceComponentPtr GuiPanel_NetworkVideoSourceComponent::getNetworkVideoSourceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<NetworkVideoSourceComponent>(component);
	}
	return nullptr;
}

void GuiPanel_NetworkVideoSourceComponent::drawCompactGui()
{
	GuiPanel_EntityAccessorPtr entityAccessor= getPropertyInterface();

	static const std::set<std::string> compactProperties= {
		MikanComponentDefinition::k_componentNamePropertyId,
		NetworkVideoSourceDefinition::k_addressPropertyId,
	};
	static const std::set<std::string> compactFunctions= {
		NetworkVideoSourceComponent::k_showVideoSourceSettingsFunctionId};
	entityAccessor->drawPropertiesGui(compactProperties);
	entityAccessor->drawFunctionsGui(compactFunctions);
}