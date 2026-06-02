#pragma once

#include "Shared/GuiPanel.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MkGuiStyle.h"
#include "ObjectSystemFwd.h"

#include <memory>

class GuiPanel_ProjectSources : public GuiPanel
{
public:
	GuiPanel_ProjectSources(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	VideoSourceComponentPtr getSelectedVideoSource() const;
	USBVideoSourceComponentPtr getSelectedUSBVideoSource() const;
	NetworkVideoSourceComponentPtr getSelectedNetworkVideoSource() const;
	TextureSourceComponentPtr getSelectedTextureSource() const;
	ClientTextureSourceComponentPtr getSelectedClientTextureSource() const;
	SpoutTextureSourceComponentPtr getSelectedSpoutTextureSource() const;
	CEFTextureSourceComponentPtr getSelectedCEFTextureSource() const;

	void setSelectedVideoSourceId(MikanVideoSourceID videoSourceId);
	void setSelectedTextureSourceId(MikanTextureSourceID textureSourceId);

	class ProjectGuiPanelContext* m_context = nullptr;
	ProjectManagerWeakPtr m_projectManager;

	int m_selectedVideoSourceId = INVALID_MIKAN_ID;
	int m_selectedTextureSourceId = INVALID_MIKAN_ID;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	std::unique_ptr<GuiDataSource_ComboBox> m_videoSourceDataSource;
	std::unique_ptr<GuiDataSource_ComboBox> m_textureSourceDataSource;
};
