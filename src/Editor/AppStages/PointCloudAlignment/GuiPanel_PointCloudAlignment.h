#pragma once

#include "Constants_PointCloudAlignment.h"
#include "NaturalFeatureCloudBuilder.h" // CloudBuildStats
#include "ModelPointCloudAligner.h"     // IcpResult
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_PointCloudAlignment : public GuiPanel
{
public:
	GuiPanel_PointCloudAlignment(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}
	virtual void onGui() override;

	ePointCloudAlignmentMenuState getMenuState() const { return m_menuState; }
	void setMenuState(ePointCloudAlignmentMenuState newState) { m_menuState= newState; }

	void setCaptureStats(const CloudBuildStats& stats) { m_captureStats= stats; }
	void setAlignmentResult(const IcpResult& result) { m_alignmentResult= result; }

	// Generic dialog events
	std::function<void()> OnOkEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;

	// Workflow-specific events
	std::function<void()> OnBeginRoiEvent;
	std::function<void()> OnSkipRoiEvent;
	std::function<void()> OnStartCaptureEvent;
	std::function<void()> OnStopCaptureEvent;
	std::function<void()> OnRunAlignmentEvent;

private:
	ePointCloudAlignmentMenuState m_menuState= ePointCloudAlignmentMenuState::inactive;
	CloudBuildStats m_captureStats;
	IcpResult m_alignmentResult;
};
