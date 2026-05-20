#include "AppStage.h"
#include "DMXObjectSystem.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Shared/GuiPanel_DMXObjectSystem.h"

#include "imgui.h"

bool GuiPanel_DMXObjectSystem::init()
{
	return initTypedPropertyInterface<DMXObjectSystem>(getOwnerAppStage());
}

void GuiPanel_DMXObjectSystem::onConstruct()
{
	if (MkGuiStyleManager* mgr = getGuiStyleManager())
		m_defaultGuiStyle = mgr->getStyle("default_component_panel");

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def = getDMXObjectSystemDefinition();
			if (!def) return false;

			char ipBuf[64];
			const std::string& ip = def->getNetworkInterfaceIP();
			strncpy_s(ipBuf, ip.c_str(), sizeof(ipBuf) - 1);
			if (ImGui::InputText("Network Interface IP", ipBuf, sizeof(ipBuf)))
			{
				const std::string newIp(ipBuf);
				addDeferredGuiEvent([this, newIp]() {
					if (auto d = getDMXObjectSystemDefinition())
						d->setNetworkInterfaceIP(newIp);
				});
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_dmxPriorityPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def = getDMXObjectSystemDefinition();
			if (!def) return false;

			int priority = (int)def->getDMXPriority();
			if (ImGui::SliderInt("DMX Priority", &priority, 0, 200))
			{
				addDeferredGuiEvent([this, priority]() {
					if (auto d = getDMXObjectSystemDefinition())
						d->setDMXPriority((uint8_t)priority);
				});
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_transmitRateHzPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def = getDMXObjectSystemDefinition();
			if (!def) return false;

			float rate = def->getTransmitRateHz();
			if (ImGui::InputFloat("Transmit Rate (Hz)", &rate, 1.0f, 10.0f, "%.1f"))
			{
				addDeferredGuiEvent([this, rate]() {
					if (auto d = getDMXObjectSystemDefinition())
						d->setTransmitRateHz(rate);
				});
			}
			return true;
		});
}

void GuiPanel_DMXObjectSystem::onGui()
{
	GuiPanel_MikanObjectSystem::onGui();
}

DMXObjectSystemPtr GuiPanel_DMXObjectSystem::getDMXObjectSystem() const
{
	MikanObjectSystemPtr objectSystem = m_objectSystem.lock();
	if (objectSystem)
		return std::static_pointer_cast<DMXObjectSystem>(objectSystem);
	return nullptr;
}

DMXObjectSystemDefinitionPtr GuiPanel_DMXObjectSystem::getDMXObjectSystemDefinition() const
{
	auto dmxSystem = getDMXObjectSystem();
	if (dmxSystem)
		return dmxSystem->getDMXObjectSystemConfig();
	return nullptr;
}
