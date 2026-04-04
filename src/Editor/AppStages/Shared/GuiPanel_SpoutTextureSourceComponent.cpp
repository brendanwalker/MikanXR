#include "AppStage.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"

#include "imgui.h"

bool GuiPanel_SpoutTextureSourceComponent::init(AppStage* ownerAppStage)
{
	m_spoutTextureSourceSystem = ownerAppStage->getObjectSystemOfType<SpoutTextureSourceSystem>();
	return initTypedPropertyInterface<SpoutTextureSourceComponent>(ownerAppStage);
}

void GuiPanel_SpoutTextureSourceComponent::update(float deltaSeconds)
{
	GuiPanel_MikanComponent::update(deltaSeconds);

	// Periodically refresh the spout sender names list
	SpoutTextureSourceSystemPtr spoutSystem = getSpoutTextureSourceSystem();
	if (spoutSystem)
	{
		m_timeSinceLastSourceListRefresh += deltaSeconds;
		if (m_timeSinceLastSourceListRefresh >= k_spoutSourceListUpdateInterval)
		{
			m_timeSinceLastSourceListRefresh = 0.0f;

			m_spoutSenderNames.clear();
			std::vector<std::string> senderNames;
			spoutSystem->getAvailableSpoutSenderNames(senderNames);
			m_spoutSenderNames = senderNames;
		}
	}
}

void GuiPanel_SpoutTextureSourceComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	auto textureSourceComp = getSpoutTextureSourceComponent();
	if (!textureSourceComp)
		return;

	ImGui::Separator();

	// Spout source dropdown
	{
		const std::string& currentSourceName = textureSourceComp->getSpoutTextureSourceDefinition()->getSpoutSource();
		const char* previewLabel = currentSourceName.empty() ? "None" : currentSourceName.c_str();

		if (ImGui::BeginCombo("Spout Source", previewLabel))
		{
			for (const std::string& senderName : m_spoutSenderNames)
			{
				const bool bSelected = (senderName == currentSourceName);
				if (ImGui::Selectable(senderName.c_str(), bSelected))
				{
					addUpdateCallback([textureSourceComp, senderName]() {
						textureSourceComp->getSpoutTextureSourceDefinition()->setSpoutSource(senderName);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

SpoutTextureSourceComponentPtr GuiPanel_SpoutTextureSourceComponent::getSpoutTextureSourceComponent() const
{
	MikanComponentPtr component = m_component.lock();
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
