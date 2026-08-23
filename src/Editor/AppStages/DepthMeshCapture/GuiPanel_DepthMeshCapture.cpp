#include "DepthMeshCapture/GuiPanel_DepthMeshCapture.h"

#include "imgui.h"

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
		ImGui::TextWrapped("Starting the video stream...");
	}
	break;

	case eDepthMeshCaptureMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Failed to start the video stream.");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::verifyCameraSetup:
	{
		ImGui::TextWrapped("Frame the part of the scene the composited character will interact with - "
						   "the floor and nearby surfaces that should catch its shadow. Thin structures "
						   "and transparent surfaces reconstruct poorly.");
		ImGui::Spacing();
		ImGui::TextWrapped("If one of the project's ArUco markers is visible in the frame, it is used to "
						   "calibrate the metric scale automatically.");
		ImGui::Spacing();
		if (!m_executionProvider.empty())
		{
			ImGui::TextWrapped("Inference backend: %s", m_executionProvider.c_str());
			if (m_executionProvider == "CPU")
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "Running on CPU - expect ~10 seconds rather than ~1.");
			}
		}
		ImGui::Spacing();
		if (ImGui::Button("Capture"))
		{
			if (OnCaptureEvent)
				OnCaptureEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::runningInference:
	{
		const char* phaseLabel= "Working...";
		int stepIndex= 1;
		switch (m_capturePhase)
		{
		case eDepthMeshCapturePhase::loadingModel:
			phaseLabel= "Loading the depth model...";
			stepIndex= 1;
			break;
		case eDepthMeshCapturePhase::runningInference:
			phaseLabel= "Estimating scene depth...";
			stepIndex= 2;
			break;
		case eDepthMeshCapturePhase::calibratingScale:
			phaseLabel= "Calibrating metric scale...";
			stepIndex= 3;
			break;
		case eDepthMeshCapturePhase::generatingMesh:
			phaseLabel= "Generating the proxy mesh...";
			stepIndex= 4;
			break;
		default:
			break;
		}

		if (m_bCancellingCapture)
			ImGui::TextWrapped("Cancelling...");
		else
			ImGui::TextWrapped("%s", phaseLabel);

		// The bar advances per step rather than smoothly: ONNX Runtime reports
		// nothing from inside a Run, and the two long steps (model load and
		// inference) are exactly the opaque ones. The elapsed time ticks every
		// frame, which is what actually shows the work is still alive.
		char overlay[32];
		snprintf(overlay, sizeof(overlay), "Step %d of %d", stepIndex, k_depthMeshCaptureStepCount);
		ImGui::ProgressBar(k_stepStartFraction[stepIndex - 1], ImVec2(-FLT_MIN, 0.f), overlay);
		ImGui::TextWrapped("%.1f s elapsed", m_captureElapsedSeconds);

		ImGui::Spacing();
		ImGui::BeginDisabled(m_bCancellingCapture);
		if (ImGui::Button("Cancel Capture"))
		{
			if (OnCancelCaptureEvent)
				OnCancelCaptureEvent();
		}
		ImGui::EndDisabled();
	}
	break;

	case eDepthMeshCaptureMenuState::failedInference:
	{
		ImGui::TextWrapped("Depth mesh capture failed.");
		ImGui::Spacing();
		if (!m_failureReason.empty())
			ImGui::TextWrapped("%s", m_failureReason.c_str());
		ImGui::Spacing();
		if (ImGui::Button("Retry"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::verifyMesh:
	{
		ImGui::TextWrapped("Proxy mesh: %d vertices, %d triangles.", m_vertexCount, m_triangleCount);
		ImGui::TextWrapped("Depth range: %.2f - %.2f m", m_nearDepth, m_farDepth);

		// Metric scale is the model's weakest output, so its correction status
		// is surfaced as prominently as the confidence numbers elsewhere.
		switch (m_scaleCorrectionSource)
		{
		case eDepthScaleCorrectionSource::arucoMarker:
			ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "Scale correction: %.3f (ArUco marker, spread %.1f%%)",
							   m_scaleCorrectionFactor, m_scaleCornerSpread * 100.f);
			if (m_scaleCornerSpread > 0.05f)
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f),
								   "The marker corners disagree on the factor - the marker may be at a "
								   "grazing angle or on a depth edge. Reposition it and redo.");
			}
			else
			{
				ImGui::TextWrapped("Creating the stencil saves this factor to the camera for future "
								   "marker-less captures.");
			}
			break;
		case eDepthScaleCorrectionSource::storedOnCamera:
			ImGui::TextWrapped("Scale correction: %.3f (stored on camera)", m_scaleCorrectionFactor);
			break;
		case eDepthScaleCorrectionSource::none:
			ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "No scale calibration");
			ImGui::TextWrapped("Metric scale is the model's raw guess and can be off by a large factor. "
							   "Place an ArUco marker of known size in view and redo to calibrate.");
			break;
		default:
			break;
		}
		if (m_culledCells > 0)
		{
			ImGui::TextWrapped("%d cells cut at depth discontinuities (expected along silhouettes - "
							   "the mesh separates rather than stretching skirts between surfaces).",
							   m_culledCells);
		}

		ImGui::Spacing();
		ImGui::TextWrapped("The overlay colors the recovered depth (red = near, blue = far). Check that "
						   "color edges hug the object silhouettes in the frame before applying.");
		ImGui::Spacing();

		if (ImGui::Button("Create Stencil"))
		{
			if (OnApplyEvent)
				OnApplyEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eDepthMeshCaptureMenuState::captureComplete:
	{
		ImGui::TextWrapped("Created model stencil '%s'.", m_createdStencilName.c_str());
		ImGui::Spacing();
		ImGui::TextWrapped("The stencil is parented under the stage at the capturing camera's pose; "
						   "connected clients pick it up automatically.");
		if (m_scaleCorrectionSource == eDepthScaleCorrectionSource::arucoMarker)
		{
			ImGui::TextWrapped("Scale correction %.3f was saved to the camera and will apply to future "
							   "captures without a marker in view.",
							   m_scaleCorrectionFactor);
		}
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
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
