#pragma once
#include "Windows/NodeEditorWindow.h"

namespace SelectedItemType
{
	// Assets
	const t_selected_item_type MATERIAL= 4;
	const t_selected_item_type MODEL= 5;
	const t_selected_item_type TEXTURE= 6;
};

class CompositorNodeEditorWindow : public NodeEditorWindow
{
public:
	CompositorNodeEditorWindow();
	~CompositorNodeEditorWindow();

	// -- NodeEditorWindow -----
	virtual NodeGraphPtr allocateNodeGraph();
	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void update(float deltaSeconds) override;
	virtual void renderToolbar() override;
	virtual void renderDragDrop(const class NodeEditorState& editorState) override;

private:
	bool m_IsPlaying = false;
	bool m_OnInit = false;
};