#pragma once

#include "Constants_AlignCameraByOriginMarker.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_AlignCameraByOriginMarker : public GuiPanel
{
public:
	GuiPanel_AlignCameraByOriginMarker(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}
	virtual void onGui() override;

	eAlignCameraByOriginMarkerMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eAlignCameraByOriginMarkerMenuState newState) { m_menuState= newState; }

	void setCaptureFraction(float fraction) { m_capturePercent= fraction * 100.f; }

	bool getMarkerVisible() const { return m_isMarkerVisible; }
	void setMarkerVisible(bool visible) { m_isMarkerVisible= visible; }

	std::function<void()> OnBeginEvent;
	std::function<void()> OnRestartEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnReturnEvent;

private:
	eAlignCameraByOriginMarkerMenuState m_menuState= eAlignCameraByOriginMarkerMenuState::inactive;
	float m_capturePercent= 0.f;
	bool m_isMarkerVisible= false;
};
