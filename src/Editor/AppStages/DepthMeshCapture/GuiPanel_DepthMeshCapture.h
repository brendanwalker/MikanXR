#pragma once

#include "Constants_DepthMeshCapture.h"
#include "Shared/GuiPanel.h"

#include <functional>
#include <string>

class GuiPanel_DepthMeshCapture : public GuiPanel
{
public:
	GuiPanel_DepthMeshCapture(class AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}
	virtual void onGui() override;

	eDepthMeshCaptureMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eDepthMeshCaptureMenuState newState) { m_menuState= newState; }

	void setExecutionProvider(const std::string& provider) { m_executionProvider= provider; }
	void setFailureReason(const std::string& reason) { m_failureReason= reason; }
	void setCreatedStencilName(const std::string& name) { m_createdStencilName= name; }

	/// Populated after a successful capture so the operator can judge the mesh
	/// before a stencil is created from it.
	void setMeshSummary(int vertexCount, int triangleCount, int culledCells, float nearDepth, float farDepth)
	{
		m_vertexCount= vertexCount;
		m_triangleCount= triangleCount;
		m_culledCells= culledCells;
		m_nearDepth= nearDepth;
		m_farDepth= farDepth;
	}

	void setScaleCorrection(eDepthScaleCorrectionSource source, float factor, float cornerSpread)
	{
		m_scaleCorrectionSource= source;
		m_scaleCorrectionFactor= factor;
		m_scaleCornerSpread= cornerSpread;
	}

	/// Pushed every frame while a capture runs so the progress readout tracks
	/// the worker.
	void setCaptureProgress(eDepthMeshCapturePhase phase, float elapsedSeconds, bool bCancelling)
	{
		m_capturePhase= phase;
		m_captureElapsedSeconds= elapsedSeconds;
		m_bCancellingCapture= bCancelling;
	}

	std::function<void()> OnCaptureEvent;
	std::function<void()> OnCancelCaptureEvent;
	std::function<void()> OnApplyEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnOkEvent;

private:
	eDepthMeshCaptureMenuState m_menuState= eDepthMeshCaptureMenuState::inactive;
	std::string m_executionProvider;
	std::string m_failureReason;
	std::string m_createdStencilName;

	int m_vertexCount= 0;
	int m_triangleCount= 0;
	int m_culledCells= 0;
	float m_nearDepth= 0.f;
	float m_farDepth= 0.f;

	eDepthScaleCorrectionSource m_scaleCorrectionSource= eDepthScaleCorrectionSource::none;
	float m_scaleCorrectionFactor= 1.f;
	float m_scaleCornerSpread= 0.f;

	eDepthMeshCapturePhase m_capturePhase= eDepthMeshCapturePhase::idle;
	float m_captureElapsedSeconds= 0.f;
	bool m_bCancellingCapture= false;
};
