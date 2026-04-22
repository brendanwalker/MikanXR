#pragma once

#include "ComponentFwd.h"
#include "Windows/NodeEditorWindow.h"

class CompositorNodeEditorWindow : public NodeEditorWindow
{
public:
	CompositorNodeEditorWindow(class App* ownerApp);

	// -- IEditorWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void shutdown() override;

	// -- NodeEditorWindow ----
	virtual NodeGraphFactoryPtr getNodeGraphFactory() const override;
	virtual bool saveGraph(bool bShowFileDialog) override;
	virtual void onNodeGraphCreated() override;

	virtual void handleGraphVariablesDragDrop(const class NodeEditorState& editorState) override;
	virtual void handleMainFrameDragDrop(const class NodeEditorState& editorState) override;

	virtual void renderToolbar() override;

	// -- CompositorNodeEditorWindow ----
	bool bindCompositorComponent(CompositorComponentPtr compositorComponent);

protected:
	CompositorComponentPtr m_compositorComponent;
	bool m_isRunningCompositor= true;
};