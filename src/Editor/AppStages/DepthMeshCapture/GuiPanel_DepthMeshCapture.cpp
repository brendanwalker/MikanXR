#include "DepthMeshCapture/GuiPanel_DepthMeshCapture.h"

#include "imgui.h"
#include "LocText.h"

#include <cfloat>
#include <cstdio>

// Fraction shown at the start of each capture step. Weighted toward the model
// load and inference steps because they dominate the wall clock; the last two
// are near-instant.
static const float k_stepStartFraction[k_depthMeshCaptureStepCount]= {0.05f, 0.25f, 0.85f, 0.92f};

void GuiPanel_DepthMeshCapture::onGui()
{
	switch (m_menuState)
	{
	case eDepthMeshCaptureMenuState::pendingVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("depthMeshCapture.startingVideoStream"));
	}
	break;

	case eDepthMeshCaptureMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("depthMeshCapture.videoStreamStartFailed"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::verifyCameraSetup:
	{
		ImGui::TextWrapped("%s", locText("depthMeshCapture.frameSceneInstructions"));
		ImGui::Spacing();
		ImGui::TextWrapped("%s", locText("depthMeshCapture.arucoAutoCalibrateNote"));
		ImGui::Spacing();
		if (!m_executionProvider.empty())
		{
			ImGui::TextWrapped("%s",
							   locFormat("depthMeshCapture.inferenceBackendFmt", m_executionProvider.c_str()).c_str());
			if (m_executionProvider == "CPU")
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "%s", locText("depthMeshCapture.cpuWarning"));
			}
		}
		ImGui::Spacing();
		if (ImGui::Button(locLabel("depthMeshCapture.capture")))
		{
			if (OnCaptureEvent)
				OnCaptureEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::runningInference:
	{
		const char* phaseLabel= locText("depthMeshCapture.working");
		int stepIndex= 1;
		switch (m_capturePhase)
		{
		case eDepthMeshCapturePhase::loadingModel:
			phaseLabel= locText("depthMeshCapture.loadingModel");
			stepIndex= 1;
			break;
		case eDepthMeshCapturePhase::runningInference:
			phaseLabel= locText("depthMeshCapture.estimatingDepth");
			stepIndex= 2;
			break;
		case eDepthMeshCapturePhase::calibratingScale:
			phaseLabel= locText("depthMeshCapture.calibratingScale");
			stepIndex= 3;
			break;
		case eDepthMeshCapturePhase::generatingMesh:
			phaseLabel= locText("depthMeshCapture.generatingMesh");
			stepIndex= 4;
			break;
		default:
			break;
		}

		if (m_bCancellingCapture)
			ImGui::TextWrapped("%s", locText("depthMeshCapture.cancelling"));
		else
			ImGui::TextWrapped("%s", phaseLabel);

		// The bar advances per step rather than smoothly: ONNX Runtime reports
		// nothing from inside a Run, and the two long steps (model load and
		// inference) are exactly the opaque ones. The elapsed time ticks every
		// frame, which is what actually shows the work is still alive.
		char overlay[32];
		snprintf(overlay, sizeof(overlay), locText("depthMeshCapture.stepOfStepsFmt"), stepIndex,
				 k_depthMeshCaptureStepCount);
		ImGui::ProgressBar(k_stepStartFraction[stepIndex - 1], ImVec2(-FLT_MIN, 0.f), overlay);
		ImGui::TextWrapped("%s", locFormat("depthMeshCapture.elapsedSecondsFmt", m_captureElapsedSeconds).c_str());

		ImGui::Spacing();
		ImGui::BeginDisabled(m_bCancellingCapture);
		if (ImGui::Button(locLabel("depthMeshCapture.cancelCapture")))
		{
			if (OnCancelCaptureEvent)
				OnCancelCaptureEvent();
		}
		ImGui::EndDisabled();
	}
	break;

	case eDepthMeshCaptureMenuState::failedInference:
	{
		ImGui::TextWrapped("%s", locText("depthMeshCapture.captureFailed"));
		ImGui::Spacing();
		if (!m_failureReason.empty())
			ImGui::TextWrapped("%s", m_failureReason.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("depthMeshCapture.retry")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::verifyMesh:
	{
		ImGui::TextWrapped("%s", locFormat("depthMeshCapture.meshSummaryFmt", m_vertexCount, m_triangleCount).c_str());
		ImGui::TextWrapped("%s", locFormat("depthMeshCapture.depthRangeFmt", m_nearDepth, m_farDepth).c_str());

		// Metric scale is the model's weakest output, so its correction status
		// is surfaced as prominently as the confidence numbers elsewhere.
		switch (m_scaleCorrectionSource)
		{
		case eDepthScaleCorrectionSource::arucoMarker:
			ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), locText("depthMeshCapture.scaleCorrectionArucoFmt"),
							   m_scaleCorrectionFactor, m_scaleCornerSpread * 100.f);
			if (m_scaleCornerSpread > 0.05f)
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "%s",
								   locText("depthMeshCapture.markerCornerDisagreement"));
			}
			else
			{
				ImGui::TextWrapped("%s", locText("depthMeshCapture.scaleFactorSavedNote"));
			}
			break;
		case eDepthScaleCorrectionSource::storedOnCamera:
			ImGui::TextWrapped("%s",
							   locFormat("depthMeshCapture.scaleCorrectionStoredFmt", m_scaleCorrectionFactor).c_str());
			break;
		case eDepthScaleCorrectionSource::none:
			ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "%s", locText("depthMeshCapture.noScaleCalibration"));
			ImGui::TextWrapped("%s", locText("depthMeshCapture.noScaleCalibrationExplanation"));
			break;
		default:
			break;
		}
		if (m_culledCells > 0)
		{
			ImGui::TextWrapped("%s", locFormat("depthMeshCapture.culledCellsFmt", m_culledCells).c_str());
		}

		ImGui::Spacing();
		ImGui::TextWrapped("%s", locText("depthMeshCapture.previewOverlayNote"));
		ImGui::Spacing();

		if (ImGui::Button(locLabel("depthMeshCapture.createStencil")))
		{
			if (OnApplyEvent)
				OnApplyEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("depthMeshCapture.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::captureComplete:
	{
		ImGui::TextWrapped("%s", locFormat("depthMeshCapture.stencilCreatedFmt", m_createdStencilName.c_str()).c_str());
		ImGui::Spacing();
		ImGui::TextWrapped("%s", locText("depthMeshCapture.stencilParentedNote"));
		if (m_scaleCorrectionSource == eDepthScaleCorrectionSource::arucoMarker)
		{
			ImGui::TextWrapped("%s",
							   locFormat("depthMeshCapture.scaleCorrectionSavedFmt", m_scaleCorrectionFactor).c_str());
		}
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
	}
	break;

	default:
		break;
	}
}
