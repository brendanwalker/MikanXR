#pragma once

// TEMP SPIKE (node-zoom branch): throwaway window evaluating imgui-node-editor
// (zoom-capable node canvas) against ImGui 1.92 and the one-ImGui-context-per-
// window setup. Delete along with the thirdparty clone when the verdict lands.

#include "EditorWindow.h"

#include <cstdint>
#include <vector>

namespace ax
{
namespace NodeEditor
{
struct EditorContext;
}
} // namespace ax

class NodeZoomSpikeWindow : public EditorWindow
{
public:
	NodeZoomSpikeWindow(class App* ownerApp);

	// -- IEditorWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual bool getIsRenderingStage() const override { return false; }
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

	// Automation readbacks (the spike verdict data)
	float getLastZoom() const { return m_lastZoom; }
	int getLinkCount() const { return (int)m_links.size(); }
	uint64_t getFrameCount() const { return m_frameCount; }

private:
	void updateUI();

	ax::NodeEditor::EditorContext* m_editorContext= nullptr;

	struct SpikeLink
	{
		int linkId;
		int startPinId;
		int endPinId;
	};
	std::vector<SpikeLink> m_links;
	int m_nextLinkId= 100;

	float m_lastZoom= 1.f;
	uint64_t m_frameCount= 0;
	bool m_bFirstFrame= true;
};
