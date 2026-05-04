#pragma once

#include "Constants_AlignCameraByUtilityMarker.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_AlignCameraByUtilityMarker : public GuiPanel
{
public:
	GuiPanel_AlignCameraByUtilityMarker(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual void onGui() override;

	eAlignCameraByUtilityMarkerMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eAlignCameraByUtilityMarkerMenuState newState) { m_menuState = newState; }

	void setSourceCaptureFraction(float fraction) { m_sourcePercent = fraction * 100.f; }
	void setTargetCaptureFraction(float fraction) { m_targetPercent = fraction * 100.f; }

	bool getSourceMarkerVisible() const { return m_isSourceMarkerVisible; }
	void setSourceMarkerVisible(bool visible) { m_isSourceMarkerVisible = visible; }

	bool getTargetMarkerVisible() const { return m_isTargetMarkerVisible; }
	void setTargetMarkerVisible(bool visible) { m_isTargetMarkerVisible = visible; }

	std::function<void()> OnBeginEvent;
	std::function<void()> OnRestartEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnReturnEvent;

private:
	eAlignCameraByUtilityMarkerMenuState m_menuState = eAlignCameraByUtilityMarkerMenuState::inactive;
	float m_sourcePercent = 0.f;
	float m_targetPercent = 0.f;
	bool m_isSourceMarkerVisible = false;
	bool m_isTargetMarkerVisible = false;
};
