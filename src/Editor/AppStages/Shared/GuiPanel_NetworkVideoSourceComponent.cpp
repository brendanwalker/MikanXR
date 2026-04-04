#include "AppStage.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceComponent.h"

#include "imgui.h"

bool GuiPanel_NetworkVideoSourceComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<NetworkVideoSourceComponent>(ownerAppStage);
}

void GuiPanel_NetworkVideoSourceComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	auto videoSourceComp = getNetworkVideoSourceComponent();
	if (!videoSourceComp)
		return;

	ImGui::Separator();

	// Protocol selection (TODO: Make a data source adapter class for this)
	auto definitionPtr= videoSourceComp->getNetworkVideoSourceDefinition();
	const eNetworkVideoProtocol currentProtocol = definitionPtr->getProtocol();
	const std::string& currentProtocolName = definitionPtr->getProtocolString();
	if (ImGui::BeginCombo("Protocol", currentProtocolName.c_str()))
	{
		for (int enumIntValue= 0; enumIntValue < (int)eNetworkVideoProtocol::COUNT; enumIntValue++)
		{
			const eNetworkVideoProtocol protocol = (eNetworkVideoProtocol)enumIntValue;
			const std::string protocolString = k_NetworkVideoProtocol[enumIntValue];
			bool selected = (protocol == currentProtocol);
			if (ImGui::Selectable(protocolString.c_str(), selected))
			{
				addUpdateCallback([videoSourceComp, protocol]() {
					videoSourceComp->getNetworkVideoSourceDefinition()->setProtocol(protocol);
				});
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

NetworkVideoSourceComponentPtr GuiPanel_NetworkVideoSourceComponent::getNetworkVideoSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<NetworkVideoSourceComponent>(component);
	}
	return nullptr;
}
