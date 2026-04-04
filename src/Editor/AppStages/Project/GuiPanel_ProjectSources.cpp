#include "GuiPanel_ProjectSources.h"
#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "MikanCoreTypes.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "ProjectManager.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "TextureSourceQueries.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceQueries.h"

#include "imgui.h"

bool GuiPanel_ProjectSources::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_projectManager = ownerAppStage->getProjectManager();
	return true;
}

void GuiPanel_ProjectSources::dispose()
{
	GuiPanel::dispose();
}

VideoSourceComponentPtr GuiPanel_ProjectSources::getSelectedVideoSource() const
{
	return VideoSourceQueries::getVideoSourceById(
		m_projectManager.lock(), (MikanVideoSourceID)m_selectedVideoSourceId);
}

USBVideoSourceComponentPtr GuiPanel_ProjectSources::getSelectedUSBVideoSource() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<USBVideoSourceSystem>();
	return sys->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

NetworkVideoSourceComponentPtr GuiPanel_ProjectSources::getSelectedNetworkVideoSource() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<NetworkVideoSourceSystem>();
	return sys->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

TextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedTextureSource() const
{
	return TextureSourceQueries::getTextureSourceById(
		m_projectManager.lock(), (MikanTextureSourceID)m_selectedTextureSourceId);
}

ClientTextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedClientTextureSource() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<ClientTextureSourceSystem>();
	return sys->getTypedComponentById((MikanTextureSourceID)m_selectedTextureSourceId);
}

SpoutTextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedSpoutTextureSource() const
{
	auto pm = m_projectManager.lock();
	auto sys = pm->getSystemOfType<SpoutTextureSourceSystem>();
	return sys->getTypedComponentById((MikanTextureSourceID)m_selectedTextureSourceId);
}

void GuiPanel_ProjectSources::setSelectedVideoSourceId(MikanVideoSourceID videoSourceId)
{
	m_selectedVideoSourceId = (int)videoSourceId;

	// Reset all video source panels first
	m_context->getUSBVideoSourcePanel()->setComponent(nullptr);
	m_context->getNetworkVideoSourcePanel()->setComponent(nullptr);

	auto pm = m_projectManager.lock();
	eVideoSourceType sourceType = VideoSourceQueries::getVideoSourceType(pm, videoSourceId);
	switch (sourceType)
	{
		case eVideoSourceType::usb:
			if (USBVideoSourceComponentPtr src = getSelectedUSBVideoSource())
				m_context->getUSBVideoSourcePanel()->setComponent(src);
			break;
		case eVideoSourceType::networked:
			if (NetworkVideoSourceComponentPtr src = getSelectedNetworkVideoSource())
				m_context->getNetworkVideoSourcePanel()->setComponent(src);
			break;
	}
}

void GuiPanel_ProjectSources::setSelectedTextureSourceId(MikanTextureSourceID textureSourceId)
{
	m_selectedTextureSourceId = (int)textureSourceId;

	// Reset all texture source panels first
	m_context->getClientTextureSourcePanel()->setComponent(nullptr);
	m_context->getSpoutTextureSourcePanel()->setComponent(nullptr);

	auto pm = m_projectManager.lock();
	eTextureSourceType sourceType = TextureSourceQueries::getTextureSourceType(pm, textureSourceId);
	switch (sourceType)
	{
		case eTextureSourceType::client:
			if (ClientTextureSourceComponentPtr src = getSelectedClientTextureSource())
				m_context->getClientTextureSourcePanel()->setComponent(src);
			break;
		case eTextureSourceType::spout:
			if (SpoutTextureSourceComponentPtr src = getSelectedSpoutTextureSource())
				m_context->getSpoutTextureSourcePanel()->setComponent(src);
			break;
	}
}

void GuiPanel_ProjectSources::render(float deltaSeconds)
{
	auto pm = m_projectManager.lock();
	if (!pm)
		return;

	// Video Sources
	ImGui::Text("Video Sources");

	VideoSourceIdList videoIds = VideoSourceQueries::getVideoSourceIdList(pm);

	// Validate selection
	bool videoSelectionValid = false;
	for (MikanVideoSourceID id : videoIds)
	{
		if ((int)id == m_selectedVideoSourceId)
		{
			videoSelectionValid = true;
			break;
		}
	}
	if (!videoSelectionValid && m_selectedVideoSourceId != INVALID_MIKAN_ID)
		setSelectedVideoSourceId(INVALID_MIKAN_ID);

	if (ImGui::BeginListBox("##VideoSources", ImVec2(-1, 80)))
	{
		for (MikanVideoSourceID id : videoIds)
		{
			VideoSourceComponentPtr source = VideoSourceQueries::getVideoSourceById(pm, id);
			std::string label = source ? (source->getName().empty()
				? "VideoSource " + std::to_string((int)id)
				: source->getName()) : "VideoSource " + std::to_string((int)id);
			label += "##vs" + std::to_string((int)id);

			bool selected = (m_selectedVideoSourceId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addUpdateCallback([this, newId]() {
					setSelectedVideoSourceId((MikanVideoSourceID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add USB Source"))
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<USBVideoSourceSystem>();
			sys->addNewUSBVideoSource();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Network Source"))
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<NetworkVideoSourceSystem>();
			sys->addNewObjectByTypedDefinition();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Video Source") && m_selectedVideoSourceId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			VideoSourceQueries::removeVideoSource(pm, (MikanVideoSourceID)m_selectedVideoSourceId);
		});
	}

	ImGui::Separator();

	// Texture Sources
	ImGui::Text("Texture Sources");

	TextureSourceIdList textureIds = TextureSourceQueries::getTextureSourceIdList(pm);

	// Validate selection
	bool textureSelectionValid = false;
	for (MikanTextureSourceID id : textureIds)
	{
		if ((int)id == m_selectedTextureSourceId)
		{
			textureSelectionValid = true;
			break;
		}
	}
	if (!textureSelectionValid && m_selectedTextureSourceId != INVALID_MIKAN_ID)
		setSelectedTextureSourceId(INVALID_MIKAN_ID);

	if (ImGui::BeginListBox("##TextureSources", ImVec2(-1, 80)))
	{
		for (MikanTextureSourceID id : textureIds)
		{
			TextureSourceComponentPtr source = TextureSourceQueries::getTextureSourceById(pm, id);
			std::string label = source ? (source->getName().empty()
				? "TextureSource " + std::to_string((int)id)
				: source->getName()) : "TextureSource " + std::to_string((int)id);
			label += "##ts" + std::to_string((int)id);

			bool selected = (m_selectedTextureSourceId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addUpdateCallback([this, newId]() {
					setSelectedTextureSourceId((MikanTextureSourceID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add Client Source"))
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<ClientTextureSourceSystem>();
			sys->addNewObjectByTypedDefinition();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Spout Source"))
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<SpoutTextureSourceSystem>();
			sys->addNewObjectByTypedDefinition();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Texture Source") && m_selectedTextureSourceId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			auto pm = m_projectManager.lock();
			TextureSourceQueries::removeTextureSource(pm, (MikanTextureSourceID)m_selectedTextureSourceId);
		});
	}
}
