#include "AppStage.h"
#include "DMXObjectSystem.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Shared/GuiPanel_DMXObjectSystem.h"

#include "imgui.h"

#include <algorithm>

bool GuiPanel_DMXObjectSystem::init() { return initTypedPropertyInterface<DMXObjectSystem>(getOwnerAppStage()); }

void GuiPanel_DMXObjectSystem::onConstruct()
{
	if (MkGuiStyleManager* mgr= getGuiStyleManager())
		m_defaultGuiStyle= mgr->getStyle("default_component_panel");

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_networkInterfaceIPPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def= getDMXObjectSystemDefinition();
			if (!def)
				return false;

			char ipBuf[64];
			const std::string& ip= def->getNetworkInterfaceIP();
			strncpy_s(ipBuf, ip.c_str(), sizeof(ipBuf) - 1);
			if (ImGui::InputText(locLabel("objectSystemPanel.networkInterfaceIP"), ipBuf, sizeof(ipBuf)))
			{
				const std::string newIp(ipBuf);
				addDeferredGuiEvent(
					[this, newIp]()
					{
						if (auto d= getDMXObjectSystemDefinition())
							d->setNetworkInterfaceIP(newIp);
					});
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_dmxPriorityPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def= getDMXObjectSystemDefinition();
			if (!def)
				return false;

			int priority= (int)def->getDMXPriority();
			if (ImGui::SliderInt(locLabel("objectSystemPanel.dmxPriority"), &priority, 0, 200))
			{
				addDeferredGuiEvent(
					[this, priority]()
					{
						if (auto d= getDMXObjectSystemDefinition())
							d->setDMXPriority((uint8_t)priority);
					});
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		DMXObjectSystemDefinition::k_transmitRateHzPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto def= getDMXObjectSystemDefinition();
			if (!def)
				return false;

			float rate= def->getTransmitRateHz();
			if (ImGui::InputFloat(locLabel("objectSystemPanel.transmitRateHz"), &rate, 1.0f, 10.0f, "%.1f"))
			{
				addDeferredGuiEvent(
					[this, rate]()
					{
						if (auto d= getDMXObjectSystemDefinition())
							d->setTransmitRateHz(rate);
					});
			}
			return true;
		});
}

void GuiPanel_DMXObjectSystem::onGui()
{
	GuiPanel_MikanObjectSystem::onGui();

	drawDestinationTable();
}

void GuiPanel_DMXObjectSystem::drawDestinationTable()
{
	auto def= getDMXObjectSystemDefinition();
	if (!def)
		return;

	ImGui::Separator();
	ImGui::TextUnformatted(locText("objectSystemPanel.dmxDestinations"));
	ImGui::TextUnformatted(locText("objectSystemPanel.dmxDestinationsHelp"));

	// Edited as a copy and written back whole, so one row's edit cannot leave the
	// table half applied while the send thread is reading it
	DMXObjectSystemDefinition::UniverseDestinationMap destinations= def->getUniverseDestinations();
	bool bChanged= false;

	int rowIndex= 0;
	for (auto it= destinations.begin(); it != destinations.end();)
	{
		ImGui::PushID(rowIndex++);

		const uint16_t universe= it->first;
		std::vector<std::string> ips= it->second;

		int universeValue= (int)universe;
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::InputInt(locLabel("objectSystemPanel.dmxUniverse"), &universeValue, 0, 0))
		{
			universeValue= std::clamp(universeValue, 1, 63999);
			if ((uint16_t)universeValue != universe && destinations.count((uint16_t)universeValue) == 0)
			{
				destinations.erase(it);
				destinations[(uint16_t)universeValue]= ips;
				bChanged= true;
				ImGui::PopID();
				break;
			}
		}

		for (size_t ipIndex= 0; ipIndex < ips.size(); ++ipIndex)
		{
			ImGui::PushID((int)ipIndex);

			char ipBuf[64];
			strncpy_s(ipBuf, ips[ipIndex].c_str(), sizeof(ipBuf) - 1);
			ImGui::SetNextItemWidth(160.f);
			if (ImGui::InputText(locLabel("objectSystemPanel.dmxDestinationIP"), ipBuf, sizeof(ipBuf)))
			{
				ips[ipIndex]= ipBuf;
				it->second= ips;
				bChanged= true;
			}

			ImGui::SameLine();
			if (ImGui::Button(locLabel("objectSystemPanel.dmxRemoveAddress")))
			{
				ips.erase(ips.begin() + ipIndex);
				it->second= ips;
				bChanged= true;
				ImGui::PopID();
				break;
			}

			ImGui::PopID();
		}

		if (ImGui::Button(locLabel("objectSystemPanel.dmxAddAddress")))
		{
			ips.push_back("");
			it->second= ips;
			bChanged= true;
		}

		ImGui::SameLine();
		if (ImGui::Button(locLabel("objectSystemPanel.dmxRemoveUniverse")))
		{
			it= destinations.erase(it);
			bChanged= true;
			ImGui::PopID();
			continue;
		}

		ImGui::Separator();
		ImGui::PopID();
		++it;
	}

	if (ImGui::Button(locLabel("objectSystemPanel.dmxAddUniverse")))
	{
		// First universe not already in the table, so the new row is immediately valid
		uint16_t universe= 1;
		while (destinations.count(universe) != 0 && universe < 63999)
			++universe;

		destinations[universe]= {std::string()};
		bChanged= true;
	}

	if (bChanged)
	{
		addDeferredGuiEvent(
			[this, destinations]()
			{
				if (auto d= getDMXObjectSystemDefinition())
					d->setUniverseDestinations(destinations);
			});
	}
}

DMXObjectSystemPtr GuiPanel_DMXObjectSystem::getDMXObjectSystem() const
{
	MikanObjectSystemPtr objectSystem= m_objectSystem.lock();
	if (objectSystem)
		return std::static_pointer_cast<DMXObjectSystem>(objectSystem);
	return nullptr;
}

DMXObjectSystemDefinitionPtr GuiPanel_DMXObjectSystem::getDMXObjectSystemDefinition() const
{
	auto dmxSystem= getDMXObjectSystem();
	if (dmxSystem)
		return dmxSystem->getDMXObjectSystemConfig();
	return nullptr;
}
