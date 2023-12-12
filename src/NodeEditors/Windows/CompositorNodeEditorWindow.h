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

	void addMaterialResource(GlMaterialPtr material);
	void deleteMaterialResource(int ix);

	void addModelResource(GlRenderModelResourcePtr modelResource);
	void deleteModelResource(int ix);

	void addTexture(GlTexturePtr texture);
	void deleteTexture(int ix);

	// -- NodeEditorWindow -----
	virtual NodeGraphPtr allocateNodeGraph();
	virtual void onNodeGraphCreated();
	virtual void onNodeGraphDeleted();
	virtual void update(float deltaSeconds) override;
	virtual void renderToolbar() override;
	virtual void renderSelectedObjectPanel() override;
	virtual void renderGraphVariablesPanel() override;
	virtual void renderDragDrop(const class NodeEditorState& editorState) override;
	virtual void deleteSelectedItem() override;

private:
	MaterialArrayPropertyPtr m_materialArrayProperty;
	ModelResourceArrayPropertyPtr m_modelResourceArrayProperty;
	TextureArrayPropertyPtr m_textureArrayProperty;

	bool m_IsPlaying = false;
	bool m_OnInit = false;
};