#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

class GuiPanel_USBVideoSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_USBVideoSourceComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void render(float deltaSeconds) override;

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
