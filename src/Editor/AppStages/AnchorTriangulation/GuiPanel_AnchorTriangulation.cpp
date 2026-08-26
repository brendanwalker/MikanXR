#include "AnchorTriangulation/GuiPanel_AnchorTriangulation.h"

#include "imgui.h"
#include "LocText.h"

void GuiPanel_AnchorTriangulation::onGui()
{
	switch (m_menuState)
	{
	case eAnchorTriangulationMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.verifyCameraSetup"));
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

	case eAnchorTriangulationMenuState::captureOrigin1:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureOrigin1"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::captureXAxis1:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureXAxis1"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::captureYAxis1:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureYAxis1"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::verifyInitialPointCapture:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.verifyInitialPointCapture"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::moveCamera:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.moveCamera"));
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

	case eAnchorTriangulationMenuState::captureOrigin2:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureOrigin2"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::captureXAxis2:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureXAxis2"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::captureYAxis2:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.captureYAxis2"));
		ImGui::Text(locText("anchorTriangulation.pointsCapturedFmt"), m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::verifyTriangulatedPoints:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.verifyTriangulatedPoints"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("anchorTriangulation.triangulationComplete"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("anchorTriangulation.redo")))
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

	case eAnchorTriangulationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("anchorTriangulation.failedVideoStream"));
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
