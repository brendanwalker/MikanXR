#pragma once

#include "Constants_LightFixtureCalibration.h"
#include "Shared/GuiPanel.h"

#include <functional>
#include <string>

class GuiPanel_LightFixtureCalibration : public GuiPanel
{
public:
	GuiPanel_LightFixtureCalibration(class AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual void onGui() override;

	eLightFixtureCalibrationMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eLightFixtureCalibrationMenuState newState) { m_menuState = newState; }

	const std::string& getFixtureName() const { return m_fixtureName; }
	void setFixtureName(const std::string& name) { m_fixtureName = name; }

	std::function<void()> OnOkEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;

private:
	eLightFixtureCalibrationMenuState m_menuState = eLightFixtureCalibrationMenuState::inactive;
	std::string m_fixtureName;
};
