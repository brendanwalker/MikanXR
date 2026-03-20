#pragma once

#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "SinglecastDelegate.h"

struct RmlModel_SceneObject
{
	Rml::String name;
	int depth;
	SelectionComponentWeakPtr selectionComponent;
};

class RmlModel_ProjectScenes : public RmlModel
{
public:
	RmlModel_ProjectScenes();

	bool init(class ProjectRmlModelContext* context);
	virtual void dispose() override;

private:
	SceneComponentPtr getSelectedSceneComponent();

	void selectSceneEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewScene(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeScene(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewAnchor(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeAnchor(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewQuad(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeQuad(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewBox(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeBox(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewModel(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeModel(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectObjectEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr);
	void onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr);

	void rebuildSceneComponentList();
	void updateSelection();
	void addTransformComponent(TransformComponentPtr transformComponentPtr, int depth);

	void setSelectedSceneId(int sceneId);

	void sceneIdListChanged(bool bOwnerChanged);
	void compositorIdListChanged(bool bOwnerChanged);

	class ProjectRmlModelContext* m_projectRmlModelContext= nullptr;
	AnchorObjectSystemWeakPtr m_anchorSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;
	EditorObjectSystemWeakPtr m_editorSystem;
	StageObjectSystemWeakPtr m_stageSystem;
	SceneObjectSystemWeakPtr m_sceneSystem;
	QuadStencilSystemWeakPtr m_quadStencilSystem;
	BoxStencilSystemWeakPtr m_boxStencilSystem;
	ModelStencilSystemWeakPtr m_modelStencilSystem;

	RmlDataBinding_ComponentIdListPtr m_sceneIdList;

	int m_selectedSceneId = -1; // MikanSceneID
	int m_selectedTransformId = -1; // MikanTransformID
	int m_selectedSceneObjectListIndex = -1;
	Rml::Vector<RmlModel_SceneObject> m_sceneOutliner;

	static bool s_bHasRegisteredTypes;
};
