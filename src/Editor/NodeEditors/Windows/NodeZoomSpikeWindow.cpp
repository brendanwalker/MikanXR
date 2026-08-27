// TEMP SPIKE (node-zoom branch): see header for scope

//-- includes -----
#include "NodeZoomSpikeWindow.h"
#include "Logger.h"
#include "MkCanvasScopedEditor.h"
#include "MkCanvasScopedNode.h"
#include "MkCanvasScopedPin.h"
#include "MkCanvasWidgets.h"
#include "MkGuiContext.h"
#include "MkGuiScopedContext.h"
#include "MkGuiScopedUpdate.h"
#include "IMkGraphicsContext.h"
#include "IMkState.h"
#include "MkScopedState.h"
#include "MkStateModifiers.h"
#include "MkStateStack.h"
#include "MkWindowEvent.h"

#include "imgui.h"
#include "imgui_node_editor.h"

#include <easy/profiler.h>

#include <algorithm>

namespace ed= ax::NodeEditor;

//-- constants -----
static const int k_spike_window_width= 900;
static const int k_spike_window_height= 600;

// Fixed ids for the three demo nodes and their pins
static const int k_nodeAId= 1;
static const int k_nodeAOutPinId= 11;
static const int k_nodeBId= 2;
static const int k_nodeBInPinId= 21;
static const int k_nodeBOutPinId= 22;
static const int k_nodeCId= 3;
static const int k_nodeCInPinId= 31;

//-- public methods -----
NodeZoomSpikeWindow::NodeZoomSpikeWindow(App* ownerApp)
	: EditorWindow(ownerApp)
{
	shareGraphicsContextWithMainWindow();
}

bool NodeZoomSpikeWindow::startup()
{
	EASY_FUNCTION();

	bool success= true;

	if (success && !startupWindow("Node Zoom Spike", k_spike_window_width, k_spike_window_height))
	{
		success= false;
	}

	if (success && !startupGuiContext("node_zoom_spike"))
	{
		success= false;
	}

	if (success && !startupStyleManager())
	{
		success= false;
	}

	if (success)
	{
		// The editor context must be created while this window's ImGui context
		// is current, since the library binds to ImGui::GetCurrentContext()
		MkGuiScopedContext scopedContext(*m_guiContext.get());

		m_editorContext= MkCanvas::createEditorContext();
		success= (m_editorContext != nullptr);
	}

	return success;
}

void NodeZoomSpikeWindow::update(float deltaSeconds)
{
	EASY_FUNCTION();

	MkGuiScopedUpdate scopedCtx(*m_guiContext);

	m_mkWindowContext->handleEvents(this);

	updateUI();
}

void NodeZoomSpikeWindow::updateUI()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(getWidth(), getHeight()));
	ImGui::Begin("Node Zoom Spike", nullptr,
				 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	ImGui::Text("zoom=%.3f  links=%d  (wheel=zoom, RMB-drag=pan)", m_lastZoom, (int)m_links.size());

	// Explicit scope: the canvas must close before the host window's End
	{
		MkCanvasScopedEditor scopedEditor(m_editorContext, "SpikeCanvas");

		const ImVec2 pinIconSize(20.f, 20.f);
		const ImVec4 pinColor(0.55f, 0.75f, 1.f, 1.f);

		// Node A: source with a button, exercising interactive widgets inside a node
		{
			MkCanvasScopedNode node(k_nodeAId, ImVec4(0.55f, 0.25f, 0.25f, 1.f));
			node.beginHeader();
			ImGui::Text("Source Node");
			node.endHeader();

			if (ImGui::Button("Do Nothing"))
			{
				MIKAN_LOG_INFO("NodeZoomSpike") << "Button clicked at zoom " << m_lastZoom;
			}
			{
				MkCanvasScopedPin pin(k_nodeAOutPinId, MkCanvasPinDirection::Output);
				ImGui::Text("out");
				ImGui::SameLine();
				MkCanvas::drawPinIcon(pinIconSize, MkCanvas::PinIcon::Circle, getLinkCount() > 0, pinColor);
			}
		}

		// Node B: pass-through with both pin kinds
		{
			MkCanvasScopedNode node(k_nodeBId, ImVec4(0.25f, 0.45f, 0.25f, 1.f));
			node.beginHeader();
			ImGui::Text("Filter Node");
			node.endHeader();

			{
				MkCanvasScopedPin pin(k_nodeBInPinId, MkCanvasPinDirection::Input);
				MkCanvas::drawPinIcon(pinIconSize, MkCanvas::PinIcon::Flow, false, pinColor);
				ImGui::SameLine();
				ImGui::Text("in");
			}
			ImGui::SameLine(90.f);
			{
				MkCanvasScopedPin pin(k_nodeBOutPinId, MkCanvasPinDirection::Output);
				ImGui::Text("out");
				ImGui::SameLine();
				MkCanvas::drawPinIcon(pinIconSize, MkCanvas::PinIcon::Circle, false, pinColor);
			}
		}

		// Node C: sink
		{
			MkCanvasScopedNode node(k_nodeCId, ImVec4(0.25f, 0.35f, 0.55f, 1.f));
			node.beginHeader();
			ImGui::Text("Output Node");
			node.endHeader();

			{
				MkCanvasScopedPin pin(k_nodeCInPinId, MkCanvasPinDirection::Input);
				MkCanvas::drawPinIcon(pinIconSize, MkCanvas::PinIcon::Grid, false, pinColor);
				ImGui::SameLine();
				ImGui::Text("in");
			}
		}

		// Existing links
		for (const SpikeLink& link : m_links)
		{
			ed::Link(link.linkId, link.startPinId, link.endPinId);
		}

		// Interactive link creation
		if (ed::BeginCreate())
		{
			ed::PinId startPinId, endPinId;
			if (ed::QueryNewLink(&startPinId, &endPinId))
			{
				if (startPinId && endPinId && ed::AcceptNewItem())
				{
					m_links.push_back({m_nextLinkId++, (int)startPinId.Get(), (int)endPinId.Get()});
				}
			}
		}
		ed::EndCreate();

		if (m_bFirstFrame)
		{
			ed::SetNodePosition(k_nodeAId, ImVec2(40.f, 100.f));
			ed::SetNodePosition(k_nodeBId, ImVec2(280.f, 180.f));
			ed::SetNodePosition(k_nodeCId, ImVec2(520.f, 120.f));
			m_bFirstFrame= false;
		}

		m_lastZoom= scopedEditor.getScreenScale();
	}

	ImGui::End();

	m_frameCount++;
}

void NodeZoomSpikeWindow::render()
{
	EASY_FUNCTION();

	m_graphicsContext->renderBegin();

	{
		MkScopedState scopedState= m_graphicsContext->getMkStateStack().createScopedState("spike renderUI");
		IMkState* glState= scopedState.getStackState();

		mkStateSetViewport(glState, 0, 0, (int)m_mkWindowContext->getWidth(), (int)m_mkWindowContext->getHeight());

		m_guiContext->submitDrawData();
	}

	m_graphicsContext->renderEnd();

	m_mkWindowContext->present();
}

void NodeZoomSpikeWindow::shutdown()
{
	if (m_editorContext != nullptr)
	{
		MkGuiScopedContext scopedContext(*m_guiContext.get());

		MkCanvas::destroyEditorContext(m_editorContext);
		m_editorContext= nullptr;
	}

	shutdownStyleManager();
	shutdownGuiContext();
	shutdownWindow();
}

// -- IMkWindowEventListener
bool NodeZoomSpikeWindow::onWindowEvent(const MkWindowEvent& event) { return m_guiContext->onWindowEvent(event); }
