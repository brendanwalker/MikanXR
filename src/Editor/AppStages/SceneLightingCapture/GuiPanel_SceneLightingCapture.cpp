#include "SceneLightingCapture/GuiPanel_SceneLightingCapture.h"

#include "imgui.h"

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
		ImGui::TextWrapped("Starting the video stream...");
	}
	break;

	case eSceneLightingCaptureMenuState::failedVideoStartStreamRequest:
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

	case eSceneLightingCaptureMenuState::verifyCameraSetup:
	{
		ImGui::TextWrapped("Frame a view of the scene that shows a good spread of surface orientations - "
						   "floor, walls and objects. Flat views of a single wall cannot constrain the "
						   "lighting.");
		ImGui::Spacing();
		ImGui::TextWrapped("Probe: %s", m_probeName.c_str());
		if (!m_executionProvider.empty())
		{
			ImGui::TextWrapped("Inference backend: %s", m_executionProvider.c_str());
			if (m_executionProvider == "CPU")
			{
				ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f),
								   "Running on CPU - this will take minutes rather than seconds.");
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

	case eSceneLightingCaptureMenuState::runningInference:
	{
		ImGui::TextWrapped("Estimating scene lighting...");
		ImGui::Spacing();
		ImGui::TextWrapped("Running two diffusion models over the captured frame. This takes several "
						   "seconds on the GPU and the window will not update until it finishes.");
	}
	break;

	case eSceneLightingCaptureMenuState::failedInference:
	{
		ImGui::TextWrapped("Lighting estimation failed.");
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

	case eSceneLightingCaptureMenuState::verifyEstimate:
	{
		ImGui::TextWrapped("Estimate recovered from %d pixels.", m_sampleCount);
		ImGui::Spacing();

		// Directionality is the confidence signal. Surfacing it prominently is
		// deliberate: a near-ambient estimate has an arbitrary key direction,
		// and presenting that as a confident result is the main way this
		// feature can mislead.
		if (m_directionality < k_lowDirectionalityThreshold)
		{
			ImGui::TextColored(ImVec4(1.f, 0.7f, 0.f, 1.f), "Directionality (l1/l0): %.3f - near ambient",
							   m_directionality);
			ImGui::TextWrapped("This scene is close to uniformly lit. The ambient term is usable, but the "
							   "key light direction is not meaningful and should not be used to place a "
							   "directional light.");
		}
		else
		{
			ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "Directionality (l1/l0): %.3f", m_directionality);
			ImGui::TextWrapped("Key light direction: %s", m_keyDirection.c_str());
		}

		ImGui::Spacing();
		ImGui::TextWrapped("Ambient: %s", m_ambient.c_str());

		if (m_negativeSolidAngleFraction > 0.01f)
		{
			ImGui::Spacing();
			ImGui::TextWrapped("Negative radiance over %.0f%% of the sphere. This is expected for "
							   "directional scenes - order-2 spherical harmonics cannot represent a sharp "
							   "light - and is clamped when the environment is used.",
							   m_negativeSolidAngleFraction * 100.f);
		}

		ImGui::Spacing();
		ImGui::TextWrapped("Compare the lit sphere against the video frame before applying.");
		ImGui::Spacing();

		if (ImGui::Button("Apply"))
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

	case eSceneLightingCaptureMenuState::captureComplete:
	{
		ImGui::TextWrapped("Lighting applied to probe '%s'.", m_probeName.c_str());
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
