#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

class GuiPanel_USBVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_USBVideoSourceComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const;
	USBVideoSourceComponentPtr getUSBVideoSourceComponent() const;

private:
	USBVideoSourceSystemWeakPtr m_usbVideoSourceSystem;
	std::vector<std::string> m_usbDevicePaths;
	std::vector<std::string> m_videoResolutions;
	std::vector<std::string> m_videoFrameRates;
	std::vector<std::string> m_videoFormats;
};
