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

void applyEditorStyle(ax::NodeEditor::EditorContext* editorContext)
{
	ax::NodeEditor::EditorContext* prevContext= ed::GetCurrentEditor();
	ed::SetCurrentEditor(editorContext);

	ed::Style& style= ed::GetStyle();
	style.NodeRounding= 6.f;
	style.NodeBorderWidth= 1.5f;
	style.HoveredNodeBorderWidth= 2.5f;
	style.SelectedNodeBorderWidth= 3.f;
	style.PinRadius= 0.f;

	style.Colors[ed::StyleColor_NodeBg]= ImColor(30, 30, 34, 240);
	style.Colors[ed::StyleColor_NodeBorder]= ImColor(62, 62, 66, 255);
	style.Colors[ed::StyleColor_HovNodeBorder]= ImColor(220, 220, 220, 200);
	style.Colors[ed::StyleColor_SelNodeBorder]= ImColor(220, 140, 0, 255); // Mikan selection orange
	style.Colors[ed::StyleColor_HovLinkBorder]= ImColor(220, 220, 220, 200);
	style.Colors[ed::StyleColor_SelLinkBorder]= ImColor(220, 140, 0, 255);

	ed::SetCurrentEditor(prevContext);
}
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
