#pragma once

#include "CommonConfigFwd.h"
#include "ComponentFwd.h"
#include "IMkGuiStyle.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectOutlinerModel.h"
#include "Shared/GuiPanel.h"

#include <set>
#include <string>
#include <vector>

class ConfigPropertyChangeSet;

// The unified project panel: a tree over every project object, and for the
// selected node a delete button, the valid add-child buttons, and the bound
// component property panel.
class GuiPanel_ProjectOutliner : public GuiPanel
{
public:
	GuiPanel_ProjectOutliner(class AppStage* ownerAppStage);

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

	// Drives the viewport render mode and selection filter in the app stage
	eOutlinerNodeKind getSelectedNodeKind() const { return m_selectedKind; }

private:
	void rebuildIfDirty();
	void markTreeDirty() { m_bTreeDirty= true; }

	// Tree drawing
	void drawTree();
	void drawNode(ProjectOutlinerNodePtr node);
	void handleNodeDragDrop(ProjectOutlinerNodePtr node);

	// Selected-node action strip and bound component panel
	void drawSelectedNodeActions(ProjectOutlinerNodePtr selectedNode);
	void drawRootAddButtons();
	void drawSceneActorAddButtons(ProjectOutlinerNodePtr selectedNode);
	void requestDeleteNode(ProjectOutlinerNodePtr node);
	int getAddParentTransformId(ProjectOutlinerNodePtr selectedNode) const;
	void deferAddAction(std::function<int(ProjectManagerPtr)> addAction);

	// Selection plumbing
	ProjectOutlinerNodePtr getSelectedNode() const;
	void setSelectedNode(ProjectOutlinerNodePtr node, bool bPushViewportSelection);
	class GuiPanel_MikanComponent* getPanelForComponentClass(const std::string& componentClassName) const;
	void clearComponentPanels();
	void drawComponentPanelForNode(ProjectOutlinerNodePtr node);

	// Active scene and active display compositor highlighting
	bool isActiveHighlightNode(ProjectOutlinerNodePtr node) const;

	// Object system delegate handlers
	void onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr);
	void onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr);
	void onSystemConfigChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void onSelectionChanged();
	void subscribeToSystems();
	void unsubscribeFromSystems();

	class ProjectGuiPanelContext* m_context= nullptr;
	ProjectManagerWeakPtr m_projectManager;
	EditorObjectSystemWeakPtr m_editorSystem;
	SceneObjectSystemWeakPtr m_sceneSystem;
	std::vector<MikanObjectSystemWeakPtr> m_subscribedSystems;

	ProjectOutlinerModel m_model;
	bool m_bTreeDirty= true;

	// Selection survives rebuilds by component id, not by node pointer
	int m_selectedComponentId= -1;
	eOutlinerNodeKind m_selectedKind= eOutlinerNodeKind::projectRoot;
	int m_pendingSelectComponentId= -1;

	// Viewport pick to tree sync: scroll to the row and open the path above it
	bool m_bScrollToSelection= false;
	std::set<int> m_scrollOpenPathIds;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	MkGuiStyleConstPtr m_outlinerGuiStyle;
};
