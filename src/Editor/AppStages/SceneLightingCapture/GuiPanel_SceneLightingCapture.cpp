#include "SceneLightingCapture/GuiPanel_SceneLightingCapture.h"

#include "imgui.h"
#include "LocText.h"

#include <cfloat>
#include <cstdio>

// Below this the recovered environment is effectively uniform ambient and the
// key light direction carries no information. Matches
// SceneLightingEstimator's warning threshold.
static constexpr float k_lowDirectionalityThreshold= 0.25f;

void GuiPanel_SceneLightingCapture::onGui()
{
	switch (m_menuState)
	{
	case eSceneLightingCaptureMenuState::pendingVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("sceneLightingCapture.startingVideoStream"));
	}
	break;

	case eSceneLightingCaptureMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("sceneLightingCapture.videoStreamStartFailed"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eSceneLightingCaptureMenuState::verifyCameraSetup:
	{
		ImGui::TextWrapped("%s", locText("sceneLightingCapture.frameSceneInstructions"));
		ImGui::Spacing();
		ImGui::TextWrapped("%s", locFormat("sceneLightingCapture.probeFmt", m_probeName.c_str()).c_str());
		if (!m_executionProvider.empty())
		{
			ImGui::TextWrapped(
				"%s", locFormat("sceneLightingCapture.inferenceBackendFmt", m_executionProvider.c_str()).c_str());
			if (m_executionProvider == "CPU")
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "%s", locText("sceneLightingCapture.cpuWarning"));
			}
		}
		ImGui::Spacing();
		if (ImGui::Button(locLabel("sceneLightingCapture.capture")))
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

	case eSceneLightingCaptureMenuState::runningInference:
	{
		const char* phaseLabel= locText("sceneLightingCapture.working");
		int stepIndex= 1;
		switch (m_estimatePhase)
		{
		case eSceneLightingEstimatePhase::loadingModels:
			phaseLabel= locText("sceneLightingCapture.loadingModels");
			stepIndex= 1;
			break;
		case eSceneLightingEstimatePhase::decomposingShading:
			phaseLabel= locText("sceneLightingCapture.decomposingShading");
			stepIndex= 2;
			break;
		case eSceneLightingEstimatePhase::estimatingGeometry:
			phaseLabel= locText("sceneLightingCapture.estimatingGeometry");
			stepIndex= 3;
			break;
		case eSceneLightingEstimatePhase::fittingLighting:
			phaseLabel= locText("sceneLightingCapture.fittingLighting");
			stepIndex= 4;
			break;
		default:
			break;
		}

		if (m_bCancellingEstimate)
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.cancelling"));
		else
			ImGui::TextWrapped("%s", phaseLabel);

		// The diffusion decomposition is a sequence of discrete pieces - a VAE
		// encode, one denoise step per scheduler timestep, then the decodes - so
		// unlike a single opaque ONNX Run it reports real sub-progress, and the
		// bar moves through the step that dominates the wall clock. The elapsed
		// clock covers the steps that cannot report anything.
		char overlay[32];
		snprintf(overlay, sizeof(overlay), locText("sceneLightingCapture.stepOfStepsFmt"), stepIndex,
				 k_sceneLightingEstimateStepCount);
		ImGui::ProgressBar(m_estimateFraction, ImVec2(-FLT_MIN, 0.f), overlay);
		ImGui::TextWrapped("%s", locFormat("sceneLightingCapture.elapsedSecondsFmt", m_estimateElapsedSeconds).c_str());

		ImGui::Spacing();
		ImGui::BeginDisabled(m_bCancellingEstimate);
		if (ImGui::Button(locLabel("sceneLightingCapture.cancelCapture")))
		{
			if (OnCancelCaptureEvent)
				OnCancelCaptureEvent();
		}
		ImGui::EndDisabled();
	}
	break;

	case eSceneLightingCaptureMenuState::failedInference:
	{
		ImGui::TextWrapped("%s", locText("sceneLightingCapture.estimationFailed"));
		ImGui::Spacing();
		if (!m_failureReason.empty())
			ImGui::TextWrapped("%s", m_failureReason.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("sceneLightingCapture.retry")))
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

	case eSceneLightingCaptureMenuState::verifyEstimate:
	{
		ImGui::TextWrapped("%s", locFormat("sceneLightingCapture.estimateRecoveredFmt", m_sampleCount).c_str());
		ImGui::Spacing();

		// Directionality is the confidence signal. Surfacing it prominently is
		// deliberate: a near-ambient estimate has an arbitrary key direction,
		// and presenting that as a confident result is the main way this
		// feature can mislead.
		if (m_directionality < k_lowDirectionalityThreshold)
		{
			ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f),
							   locText("sceneLightingCapture.directionalityNearAmbientFmt"), m_directionality);
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.nearAmbientExplanation"));
		}
		else
		{
			ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), locText("sceneLightingCapture.directionalityFmt"),
							   m_directionality);
			ImGui::TextWrapped("%s",
							   locFormat("sceneLightingCapture.keyLightDirectionFmt", m_keyDirection.c_str()).c_str());
		}

		ImGui::Spacing();
		ImGui::TextWrapped("%s", locFormat("sceneLightingCapture.ambientFmt", m_ambient.c_str()).c_str());

		if (m_negativeSolidAngleFraction > 0.01f)
		{
			ImGui::Spacing();
			ImGui::TextWrapped(
				"%s",
				locFormat("sceneLightingCapture.negativeRadianceFmt", m_negativeSolidAngleFraction * 100.f).c_str());
		}

		ImGui::Spacing();

		int previewIndex= (int)m_previewMode;
		const char* k_previewLabels[]= {
			locText("sceneLightingCapture.previewRecoveredLighting"), locText("sceneLightingCapture.previewRelitScene"),
			locText("sceneLightingCapture.previewModelShading"), locText("sceneLightingCapture.previewLitSphere")};
		if (ImGui::Combo(locLabel("sceneLightingCapture.preview"), &previewIndex, k_previewLabels,
						 IM_ARRAYSIZE(k_previewLabels)))
		{
			m_previewMode= (eLightingPreviewMode)previewIndex;
		}

		switch (m_previewMode)
		{
		case eLightingPreviewMode::recoveredLighting:
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.previewRecoveredLightingExplanation"));
			break;
		case eLightingPreviewMode::relitScene:
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.previewRelitSceneExplanation"));
			break;
		case eLightingPreviewMode::modelShading:
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.previewModelShadingExplanation"));
			break;
		case eLightingPreviewMode::litSphere:
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.previewLitSphereExplanation"));
			break;
		default:
			break;
		}

		// The one reading that is easy to get wrong, so it is stated rather
		// than left to be discovered: a probe carries no visibility term, so
		// missing cast shadows are structural and not a bad estimate.
		if (m_previewMode != eLightingPreviewMode::litSphere)
		{
			ImGui::TextWrapped("%s", locText("sceneLightingCapture.noCastShadowsNote"));
		}

		ImGui::Spacing();

		if (ImGui::Button(locLabel("sceneLightingCapture.apply")))
		{
			if (OnApplyEvent)
				OnApplyEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("sceneLightingCapture.redo")))
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

	case eSceneLightingCaptureMenuState::captureComplete:
	{
		ImGui::TextWrapped("%s", locFormat("sceneLightingCapture.lightingAppliedFmt", m_probeName.c_str()).c_str());
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
