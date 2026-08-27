#include "MkCanvasScopedEditor.h"
#include "MkGuiDockspace.h"

#include "imgui.h"
#include "imgui_node_editor.h"

#include <algorithm>

namespace ed= ax::NodeEditor;

namespace MkCanvas
{
ax::NodeEditor::EditorContext* createEditorContext()
{
	ed::Config config;
	config.SettingsFile= ""; // node positions persist in the graph config instead

	return ed::CreateEditor(&config);
}

void destroyEditorContext(ax::NodeEditor::EditorContext* editorContext) { ed::DestroyEditor(editorContext); }
} // namespace MkCanvas

MkCanvasScopedEditor::MkCanvasScopedEditor(ax::NodeEditor::EditorContext* editorContext, const char* canvasId)
{
	ed::SetCurrentEditor(editorContext);
	ed::Begin(canvasId, ImVec2(0.f, 0.f));

	// GetCurrentZoom() is the view's InvScale; its reciprocal is the on-screen
	// magnification. Baking glyphs at that density keeps logical metrics while
	// the canvas's geometric upscale samples a matching-resolution bitmap.
	const float invScale= ed::GetCurrentZoom();
	m_screenScale= (invScale > 0.f) ? (1.f / invScale) : 1.f;

	const float density= std::min(std::max(m_screenScale, 0.5f), 4.f);
	m_previousFontDensity= MkGui::setFontRasterizerDensity(density);
}

MkCanvasScopedEditor::~MkCanvasScopedEditor()
{
	MkGui::setFontRasterizerDensity(m_previousFontDensity);

	ed::End();
	ed::SetCurrentEditor(nullptr);
}
