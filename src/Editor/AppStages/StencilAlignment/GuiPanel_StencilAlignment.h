#pragma once

#include "Constants_StencilAlignment.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_StencilAlignment : public GuiPanel
{
public:
	bool init(AppStage* ownerAppStage);
	virtual void onGui() override;

	eStencilAlignmentMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eStencilAlignmentMenuState newState) { m_menuState = newState; }

	std::function<void()> OnOkEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;

private:
	eStencilAlignmentMenuState m_menuState = eStencilAlignmentMenuState::inactive;
};
