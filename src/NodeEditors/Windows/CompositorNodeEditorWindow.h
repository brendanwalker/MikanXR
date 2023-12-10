#pragma once
#include "Windows/NodeEditorWindow.h"

namespace SelectedItemType
{
	// Assets
	const t_selected_item_type TRI_MESH= 3;
	const t_selected_item_type TEXTURE= 4;
};

class CompositorNodeEditorWindow : public NodeEditorWindow
{
public:
	CompositorNodeEditorWindow();
	~CompositorNodeEditorWindow();

	void addTriangulatedMesh(GlTriangulatedMeshPtr triMesh);
	void deleteTriangulatedMesh(int ix);

	void addTexture(GlTexturePtr texture);
	void deleteTexture(int ix);

	// -- NodeEditorWindow -----
	virtual NodeGraphPtr allocateNodeGraph();
	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void update(float deltaSeconds) override;
	virtual void renderToolbar() override;
	virtual void renderRightPanel() override;
	virtual void renderLeftPanel() override;
	virtual void renderBottomPanel() override;
	virtual void renderDragDrop(const class NodeEditorState& editorState) override;
	virtual void deleteSelectedItem() override;

private:
	TriMeshArrayPropertyPtr m_triMeshArrayProperty;
	TextureArrayPropertyPtr m_textureArrayProperty;

	bool m_IsPlaying = false;
	bool m_OnInit = false;
};