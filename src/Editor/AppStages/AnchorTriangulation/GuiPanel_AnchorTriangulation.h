#pragma once

#include "Constants_AnchorTriangulation.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_AnchorTriangulation : public GuiPanel
{
public:
	GuiPanel_AnchorTriangulation(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual void onGui() override;

	bool getBypassCalibrationFlag() const { return m_bypassCalibrationFlag; }
	void setBypassCalibrationFlag(bool flag) { m_bypassCalibrationFlag = flag; }

	eAnchorTriangulationMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eAnchorTriangulationMenuState newState) { m_menuState = newState; }

	int getCapturedPointCount() const { return m_capturedPointCount; }
	void setCapturedPointCount(int count) { m_capturedPointCount = count; }

	std::function<void()> OnOkEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;

private:
	eAnchorTriangulationMenuState m_menuState = eAnchorTriangulationMenuState::inactive;
	int m_capturedPointCount = 0;
	bool m_bypassCalibrationFlag = false;
};
