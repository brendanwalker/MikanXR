#include "AnchorTriangulation/GuiPanel_AnchorTriangulation.h"

#include "imgui.h"

void GuiPanel_AnchorTriangulation::onGui()
{
	switch (m_menuState)
	{
	case eAnchorTriangulationMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped("Verify that the camera is positioned correctly and the video stream is running.");
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAnchorTriangulationMenuState::captureOrigin1:
	{
		ImGui::TextWrapped("Camera position 1: Click the anchor origin point in the image.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::captureXAxis1:
	{
		ImGui::TextWrapped("Camera position 1: Click a point along the anchor X axis.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::captureYAxis1:
	{
		ImGui::TextWrapped("Camera position 1: Click a point along the anchor Y axis.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::verifyInitialPointCapture:
	{
		ImGui::TextWrapped("Verify the captured points from position 1 look correct.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
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

	case eAnchorTriangulationMenuState::moveCamera:
	{
		ImGui::TextWrapped("Move the camera to a second position with a different angle to the anchor.");
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAnchorTriangulationMenuState::captureOrigin2:
	{
		ImGui::TextWrapped("Camera position 2: Click the anchor origin point in the image.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::captureXAxis2:
	{
		ImGui::TextWrapped("Camera position 2: Click a point along the anchor X axis.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::captureYAxis2:
	{
		ImGui::TextWrapped("Camera position 2: Click a point along the anchor Y axis.");
		ImGui::Text("Points captured: %d", m_capturedPointCount);
		ImGui::Spacing();
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

	case eAnchorTriangulationMenuState::verifyTriangulatedPoints:
	{
		ImGui::TextWrapped("Verify the triangulated anchor points look correct.");
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
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

	case eAnchorTriangulationMenuState::testCalibration:
	{
		ImGui::Text("Anchor triangulation complete!");
		ImGui::Spacing();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
	}
	break;

	case eAnchorTriangulationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Error: Failed to start video stream.");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
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
