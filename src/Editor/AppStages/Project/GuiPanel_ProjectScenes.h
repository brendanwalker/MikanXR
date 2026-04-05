#pragma once

#include "Shared/GuiPanel.h"
#include "AnchorObjectSystem.h"
#include "CommonConfigFwd.h"
#include "BoxStencilSystem.h"
#include "ComponentFwd.h"
#include "EditorObjectSystem.h"
#include "ModelStencilSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "QuadStencilSystem.h"
#include "SceneObjectSystem.h"

struct SceneOutlinerEntry
{
	std::string name;
	int depth;
	SelectionComponentWeakPtr selectionComponent;
};

class GuiPanel_ProjectScenes : public GuiPanel
{
public:
	GuiPanel_ProjectScenes(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	SceneComponentPtr getSelectedSceneComponent() const;
	CompositorComponentPtr getSelectedCompositor() const;

	void setSelectedSceneId(int sceneId);
	void setSelectedCompositorId(MikanCompositorID compositorId);
	void setSelectedSceneObject(MikanObjectPtr selectedObject);

	void rebuildSceneOutliner();
	void updateSelection();
	void addTransformComponent(TransformComponentPtr transformComponentPtr, int depth);

	// Delegate handlers
	void onAnchorSystemConfigChanged(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onStencilSystemConfigChanged(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr);
	void onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr);
	void onSelectionChanged();

	class ProjectGuiPanelContext* m_context = nullptr;
	AnchorObjectSystemWeakPtr m_anchorSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;
	EditorObjectSystemWeakPtr m_editorSystem;
	StageObjectSystemWeakPtr m_stageSystem;
	SceneObjectSystemWeakPtr m_sceneSystem;
	QuadStencilSystemWeakPtr m_quadStencilSystem;
	BoxStencilSystemWeakPtr m_boxStencilSystem;
	ModelStencilSystemWeakPtr m_modelStencilSystem;

	int m_selectedSceneId = INVALID_MIKAN_ID;
	int m_selectedCompositorId = INVALID_MIKAN_ID;
	int m_selectedTransformId = INVALID_MIKAN_ID;
	int m_selectedSceneObjectListIndex = -1;

	std::vector<SceneOutlinerEntry> m_sceneOutliner;
};
