#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

class GuiPanel_USBVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_USBVideoSourceComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	// Draw a compact version of the GUI that only shows the device path and video mode
	void drawCompactGui();

	//-- GuiPanel_MikanComponent Interface
	virtual bool init() override;
	virtual void onConstruct() override;
	virtual void onGui() override;

protected:
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const;
	USBVideoSourceComponentPtr getUSBVideoSourceComponent() const;

private:
	USBVideoSourceSystemWeakPtr m_usbVideoSourceSystem;
	GuiDataSource_StringList m_devicePathDataSource;
	GuiDataSource_StringList m_resolutionDataSource;
	GuiDataSource_StringList m_frameRateDataSource;
	GuiDataSource_StringList m_formatDataSource;

};
