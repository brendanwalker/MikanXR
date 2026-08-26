#include "StencilAlignment/GuiPanel_StencilAlignment.h"
#include "LocText.h"

#include "imgui.h"

void GuiPanel_StencilAlignment::onGui()
{
	switch (m_menuState)
	{
	case eStencilAlignmentMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.startingVideoStream"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eStencilAlignmentMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.verifyInitialCameraSetup"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eStencilAlignmentMenuState::captureOriginPixel:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureOriginPixel"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureOriginVertex:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureOriginVertex"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureXAxisPixel:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureXAxisPixel"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureXAxisVertex:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureXAxisVertex"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureYAxisPixel:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureYAxisPixel"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureYAxisVertex:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureYAxisVertex"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureZAxisPixel:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureZAxisPixel"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::captureZAxisVertex:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.captureZAxisVertex"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::verifyPointsCapture:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.verifyPointsCapture"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
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

	case eStencilAlignmentMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("stencilAlignment.alignmentComplete"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("stencilAlignment.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
	}
	break;

	case eStencilAlignmentMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("stencilAlignment.failedVideoStart"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	default:
		break;
	}
}
