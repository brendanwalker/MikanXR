#pragma once

#include "MkGuiExport.h"

namespace ax
{
namespace NodeEditor
{
struct EditorContext;
}
} // namespace ax

// Owned lifetime helpers for a node-canvas editor context (one per graph view;
// layout persistence stays off, node positions live in the graph config)
namespace MkCanvas
{
MIKAN_GUI_FUNC(ax::NodeEditor::EditorContext*) createEditorContext();
MIKAN_GUI_FUNC(void) destroyEditorContext(ax::NodeEditor::EditorContext* editorContext);

// Applies the Mikan canvas look to an editor context: node rounding, border
// weights, and the editor's hover/selection accent colors
MIKAN_GUI_FUNC(void) applyEditorStyle(ax::NodeEditor::EditorContext* editorContext);
} // namespace MkCanvas

// Binds the editor context and brackets one canvas frame (SetCurrentEditor +
// Begin/End). While the scope is open, glyphs bake at the canvas's on-screen
// magnification so text stays crisp under zoom.
class MIKAN_GUI_CLASS MkCanvasScopedEditor
{
public:
	MkCanvasScopedEditor(ax::NodeEditor::EditorContext* editorContext, const char* canvasId);
	~MkCanvasScopedEditor();

	MkCanvasScopedEditor(const MkCanvasScopedEditor&)= delete;
	MkCanvasScopedEditor& operator=(const MkCanvasScopedEditor&)= delete;

	// On-screen magnification of the canvas (1 = 100%)
	float getScreenScale() const { return m_screenScale; }

private:
	float m_screenScale= 1.f;
	float m_previousFontDensity= 1.f;
};
