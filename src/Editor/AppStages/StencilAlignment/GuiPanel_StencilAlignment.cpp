#include "StencilAlignment/GuiPanel_StencilAlignment.h"

#include "imgui.h"

void GuiPanel_StencilAlignment::onGui()
{
	switch (m_menuState)
	{
		case eStencilAlignmentMenuState::pendingVideoStart:
		{
			ImGui::TextWrapped("Starting video stream...");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::verifyInitialCameraSetup:
		{
			ImGui::TextWrapped("Verify that the camera is positioned and video stream is running.");
			ImGui::Spacing();
			if (ImGui::Button("Ok")) { if (OnOkEvent) OnOkEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureOriginPixel:
		{
			ImGui::TextWrapped("Click the pixel in the camera image that corresponds to the stencil origin.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureOriginVertex:
		{
			ImGui::TextWrapped("Click the vertex on the stencil mesh that corresponds to the origin.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureXAxisPixel:
		{
			ImGui::TextWrapped("Click the pixel in the camera image along the X axis direction.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureXAxisVertex:
		{
			ImGui::TextWrapped("Click the vertex on the stencil mesh along the X axis.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureYAxisPixel:
		{
			ImGui::TextWrapped("Click the pixel in the camera image along the Y axis direction.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureYAxisVertex:
		{
			ImGui::TextWrapped("Click the vertex on the stencil mesh along the Y axis.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureZAxisPixel:
		{
			ImGui::TextWrapped("Click the pixel in the camera image along the Z axis direction.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::captureZAxisVertex:
		{
			ImGui::TextWrapped("Click the vertex on the stencil mesh along the Z axis.");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::verifyPointsCapture:
		{
			ImGui::TextWrapped("Verify the captured alignment points look correct.");
			ImGui::Spacing();
			if (ImGui::Button("Ok")) { if (OnOkEvent) OnOkEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eStencilAlignmentMenuState::testCalibration:
		{
			ImGui::Text("Stencil alignment complete!");
			ImGui::Spacing();
			if (ImGui::Button("Redo")) { if (OnRedoEvent) OnRedoEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnOkEvent) OnOkEvent(); }
		} break;

		case eStencilAlignmentMenuState::failedVideoStartStreamRequest:
		{
			ImGui::TextWrapped("Error: Failed to start video stream.");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		default:
			break;
	}
}
