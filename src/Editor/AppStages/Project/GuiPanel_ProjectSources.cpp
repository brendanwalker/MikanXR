#include "GuiPanel_ProjectSources.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "CEFTextureSourceComponent.h"
#include "CEFTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "MikanCoreTypes.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "IconsForkAwesome.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "ProjectManager.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "Shared/GuiPanel_ShapeComponent.h"
#include "ShapeUtils.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "Shared/GuiPanel_CEFTextureSourceComponent.h"
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
	m_context= context;
	AppStage_Project* ownerAppStage= context->getOwnerAppStage();
	m_projectManager= ownerAppStage->getProjectManager();

	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");

	auto pm= ownerAppStage->getProjectManager();
	m_videoSourceDataSource= std::make_unique<GuiDataSource_ComboBox>(pm,
																	  std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
																		  {USBVideoSourceSystem::k_objectSystemClassName, USBVideoSourceComponent::k_componentClassName},
																		  {NetworkVideoSourceSystem::k_objectSystemClassName, NetworkVideoSourceComponent::k_componentClassName}});

	m_textureSourceDataSource= std::make_unique<GuiDataSource_ComboBox>(pm,
																		std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
																			{ClientTextureSourceSystem::k_objectSystemClassName, ClientTextureSourceComponent::k_componentClassName},
																			{SpoutTextureSourceSystem::k_objectSystemClassName, SpoutTextureSourceComponent::k_componentClassName},
																			{CEFTextureSourceSystem::k_objectSystemClassName, CEFTextureSourceComponent::k_componentClassName}});

	m_shapeDataSource= std::make_unique<GuiDataSource_ComboBox>(pm,
																std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
																	{QuadShapeSystem::k_objectSystemClassName, QuadShapeComponent::k_componentClassName},
																	{BoxShapeSystem::k_objectSystemClassName, BoxShapeComponent::k_componentClassName},
																	{ModelShapeSystem::k_objectSystemClassName, ModelShapeComponent::k_componentClassName}});

	// Auto-select first available video source
	m_videoSourceDataSource->refreshEntries();
	if (m_videoSourceDataSource->getEntryCount() > 0)
	{
		if (MikanComponentPtr first= m_videoSourceDataSource->getEntryAtIndex(0))
			setSelectedVideoSourceId((MikanVideoSourceID)first->getComponentId());
	}

	// Auto-select first available texture source
	m_textureSourceDataSource->refreshEntries();
	if (m_textureSourceDataSource->getEntryCount() > 0)
	{
		if (MikanComponentPtr first= m_textureSourceDataSource->getEntryAtIndex(0))
			setSelectedTextureSourceId((MikanTextureSourceID)first->getComponentId());
	}

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
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<USBVideoSourceSystem>();
	return sys->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

NetworkVideoSourceComponentPtr GuiPanel_ProjectSources::getSelectedNetworkVideoSource() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<NetworkVideoSourceSystem>();
	return sys->getTypedComponentById((MikanVideoSourceID)m_selectedVideoSourceId);
}

TextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedTextureSource() const
{
	return TextureSourceQueries::getTextureSourceById(
		m_projectManager.lock(), (MikanTextureSourceID)m_selectedTextureSourceId);
}

ClientTextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedClientTextureSource() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<ClientTextureSourceSystem>();
	return sys->getTypedComponentById((MikanTextureSourceID)m_selectedTextureSourceId);
}

SpoutTextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedSpoutTextureSource() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<SpoutTextureSourceSystem>();
	return sys->getTypedComponentById((MikanTextureSourceID)m_selectedTextureSourceId);
}

CEFTextureSourceComponentPtr GuiPanel_ProjectSources::getSelectedCEFTextureSource() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<CEFTextureSourceSystem>();
	return sys->getTypedComponentById((MikanTextureSourceID)m_selectedTextureSourceId);
}

QuadShapeComponentPtr GuiPanel_ProjectSources::getSelectedQuadShape() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<QuadShapeSystem>();
	return sys->getQuadShapeById((MikanShapeID)m_selectedShapeId);
}

BoxShapeComponentPtr GuiPanel_ProjectSources::getSelectedBoxShape() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<BoxShapeSystem>();
	return sys->getBoxShapeById((MikanShapeID)m_selectedShapeId);
}

ModelShapeComponentPtr GuiPanel_ProjectSources::getSelectedModelShape() const
{
	auto pm= m_projectManager.lock();
	auto sys= pm->getSystemOfType<ModelShapeSystem>();
	return sys->getModelShapeById((MikanShapeID)m_selectedShapeId);
}

void GuiPanel_ProjectSources::setSelectedVideoSourceId(MikanVideoSourceID videoSourceId)
{
	m_selectedVideoSourceId= (int)videoSourceId;

	m_context->getUSBVideoSourcePanel()->setComponent(nullptr);
	m_context->getNetworkVideoSourcePanel()->setComponent(nullptr);

	auto pm= m_projectManager.lock();
	eVideoSourceType sourceType= VideoSourceQueries::getVideoSourceType(pm, videoSourceId);
	switch (sourceType)
	{
	case eVideoSourceType::usb:
		if (USBVideoSourceComponentPtr src= getSelectedUSBVideoSource())
			m_context->getUSBVideoSourcePanel()->setComponent(src);
		break;
	case eVideoSourceType::networked:
		if (NetworkVideoSourceComponentPtr src= getSelectedNetworkVideoSource())
			m_context->getNetworkVideoSourcePanel()->setComponent(src);
		break;
	}
}

void GuiPanel_ProjectSources::setSelectedTextureSourceId(MikanTextureSourceID textureSourceId)
{
	m_selectedTextureSourceId= (int)textureSourceId;

	m_context->getClientTextureSourcePanel()->setComponent(nullptr);
	m_context->getSpoutTextureSourcePanel()->setComponent(nullptr);
	m_context->getCEFTextureSourcePanel()->setComponent(nullptr);

	auto pm= m_projectManager.lock();
	eTextureSourceType sourceType= TextureSourceQueries::getTextureSourceType(pm, textureSourceId);
	switch (sourceType)
	{
	case eTextureSourceType::client:
		if (ClientTextureSourceComponentPtr src= getSelectedClientTextureSource())
			m_context->getClientTextureSourcePanel()->setComponent(src);
		break;
	case eTextureSourceType::spout:
		if (SpoutTextureSourceComponentPtr src= getSelectedSpoutTextureSource())
			m_context->getSpoutTextureSourcePanel()->setComponent(src);
		break;
	case eTextureSourceType::cef:
		if (CEFTextureSourceComponentPtr src= getSelectedCEFTextureSource())
			m_context->getCEFTextureSourcePanel()->setComponent(src);
		break;
	}
}

void GuiPanel_ProjectSources::setSelectedShapeId(MikanShapeID shapeId)
{
	m_selectedShapeId= (int)shapeId;

	m_context->getQuadShapePanel()->setComponent(nullptr);
	m_context->getBoxShapePanel()->setComponent(nullptr);
	m_context->getModelShapePanel()->setComponent(nullptr);

	auto pm= m_projectManager.lock();
	eShapeType shapeType= ShapeUtils::getShapeType(pm, shapeId);
	switch (shapeType)
	{
	case eShapeType::quad:
		if (QuadShapeComponentPtr src= getSelectedQuadShape())
			m_context->getQuadShapePanel()->setComponent(src);
		break;
	case eShapeType::box:
		if (BoxShapeComponentPtr src= getSelectedBoxShape())
			m_context->getBoxShapePanel()->setComponent(src);
		break;
	case eShapeType::model:
		if (ModelShapeComponentPtr src= getSelectedModelShape())
			m_context->getModelShapePanel()->setComponent(src);
		break;
	}
}

void GuiPanel_ProjectSources::onGui()
{
	auto pm= m_projectManager.lock();
	if (!pm)
		return;

	// Video Sources
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addUSBSource", "add_usb_source"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<USBVideoSourceSystem>();
			sys->addNewUSBVideoSource(); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addNetworkSource", "add_network_source"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<NetworkVideoSourceSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}

	m_videoSourceDataSource->refreshEntries();

	if (m_selectedVideoSourceId != INVALID_MIKAN_ID &&
		m_videoSourceDataSource->getEntryIndexByComponentId(m_selectedVideoSourceId) == -1)
	{
		setSelectedVideoSourceId(INVALID_MIKAN_ID);
	}

	int videoIndex= m_videoSourceDataSource->getEntryIndexByComponentId(m_selectedVideoSourceId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectVideoSource", "Video Source",
									m_videoSourceDataSource.get(), videoIndex))
	{
		if (videoIndex >= 0)
		{
			if (MikanComponentPtr sel= m_videoSourceDataSource->getEntryAtIndex(videoIndex))
			{
				int newId= sel->getComponentId();
				addDeferredGuiEvent([this, newId]()
									{ setSelectedVideoSourceId((MikanVideoSourceID)newId); });
			}
		}
	}

	if (m_selectedVideoSourceId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeVideoSource", "delete_component"))
		{
			addDeferredGuiEvent([this]()
								{
				auto pm = m_projectManager.lock();
				VideoSourceQueries::removeVideoSource(pm, (MikanVideoSourceID)m_selectedVideoSourceId); });
		}

		m_context->getUSBVideoSourcePanel()->drawCompactGui();
		m_context->getNetworkVideoSourcePanel()->drawCompactGui();
	}

	ImGui::Separator();

	// Texture Sources
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addClientSource", "add_client_source"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<ClientTextureSourceSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addSpoutSource", "add_spout_source"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<SpoutTextureSourceSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addCEFSource", "add_cef_source"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<CEFTextureSourceSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}

	m_textureSourceDataSource->refreshEntries();

	if (m_selectedTextureSourceId != INVALID_MIKAN_ID &&
		m_textureSourceDataSource->getEntryIndexByComponentId(m_selectedTextureSourceId) == -1)
	{
		setSelectedTextureSourceId(INVALID_MIKAN_ID);
	}

	int textureIndex= m_textureSourceDataSource->getEntryIndexByComponentId(m_selectedTextureSourceId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectTextureSource", "Texture Source",
									m_textureSourceDataSource.get(), textureIndex))
	{
		if (textureIndex >= 0)
		{
			if (MikanComponentPtr sel= m_textureSourceDataSource->getEntryAtIndex(textureIndex))
			{
				int newId= sel->getComponentId();
				addDeferredGuiEvent([this, newId]()
									{ setSelectedTextureSourceId((MikanTextureSourceID)newId); });
			}
		}
	}

	if (m_selectedTextureSourceId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeTextureSource", "delete_component"))
		{
			addDeferredGuiEvent([this]()
								{
				auto pm = m_projectManager.lock();
				TextureSourceQueries::removeTextureSource(pm, (MikanTextureSourceID)m_selectedTextureSourceId); });
		}

		m_context->getClientTextureSourcePanel()->onGui();
		m_context->getSpoutTextureSourcePanel()->onGui();
		m_context->getCEFTextureSourcePanel()->onGui();
	}

	ImGui::Separator();

	// Shapes
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addQuadShape", "add_quad_shape"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<QuadShapeSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addBoxShape", "add_box_shape"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<BoxShapeSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addModelShape", "add_model_shape"))
	{
		addDeferredGuiEvent([this]()
							{
			auto pm = m_projectManager.lock();
			auto sys = pm->getSystemOfType<ModelShapeSystem>();
			sys->addNewObjectByTypedDefinition(); });
	}

	m_shapeDataSource->refreshEntries();

	if (m_selectedShapeId != INVALID_MIKAN_ID &&
		m_shapeDataSource->getEntryIndexByComponentId(m_selectedShapeId) == -1)
	{
		setSelectedShapeId(INVALID_MIKAN_ID);
	}

	int shapeIndex= m_shapeDataSource->getEntryIndexByComponentId(m_selectedShapeId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectShape", "Shape",
									m_shapeDataSource.get(), shapeIndex))
	{
		if (shapeIndex >= 0)
		{
			if (MikanComponentPtr sel= m_shapeDataSource->getEntryAtIndex(shapeIndex))
			{
				int newId= sel->getComponentId();
				addDeferredGuiEvent([this, newId]()
									{ setSelectedShapeId((MikanShapeID)newId); });
			}
		}
	}

	if (m_selectedShapeId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeShape", "delete_component"))
		{
			addDeferredGuiEvent([this]()
								{
				auto pm = m_projectManager.lock();
				ShapeUtils::removeShape(pm, (MikanShapeID)m_selectedShapeId); });
		}

		m_context->getQuadShapePanel()->onGui();
		m_context->getBoxShapePanel()->onGui();
		m_context->getModelShapePanel()->onGui();
	}
}
