#pragma once

#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_ComponentList.h"
#include "Shared/RmlModel_PropertyInterface.h"
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

	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		CompositorObjectSystemPtr compositorSystemPtr,
		EditorObjectSystemPtr editorSystemPtr,
		SceneObjectSystemPtr sceneSystemPtr,
		StageObjectSystemPtr stageSystemPtr,
		StencilObjectSystemPtr stencilSystemPtr);
	virtual void dispose() override;

private:
	StageComponentPtr getSelectedStageComponent();
	SceneComponentPtr getSelectedSceneComponent();
	CompositorComponentPtr getSelectedCompositorComponent();

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

	void setSelectedStageId(int stageId);
	void setSelectedSceneId(int sceneId);
	void setSelectedCompositorId(int compositorId);

	void stageIdListChanged(bool bOwnerChanged);
	void sceneIdListChanged(bool bOwnerChanged);
	void compositorIdListChanged(bool bOwnerChanged);

	AnchorObjectSystemWeakPtr m_anchorSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;
	EditorObjectSystemWeakPtr m_editorSystem;
	StageObjectSystemWeakPtr m_stageSystem;
	SceneObjectSystemWeakPtr m_sceneSystem;
	StencilObjectSystemWeakPtr m_stencilSystem;

	RmlDataBinding_ComponentListPtr m_stageIdList;
	RmlDataBinding_ComponentListPtr m_sceneIdList;
	RmlDataBinding_ComponentListPtr m_compositorIdList;
	RmlModel_PropertyInterfacePtr m_selectedAnchorModel;
	RmlModel_PropertyInterfacePtr m_selectedCompositorModel;
	RmlModel_PropertyInterfacePtr m_selectedBoxStencilModel;
	RmlModel_PropertyInterfacePtr m_selectedModelStencilModel;
	RmlModel_PropertyInterfacePtr m_selectedQuadStencilModel;
	RmlModel_PropertyInterfacePtr m_selectedSceneModel;

	int m_selectedStageId = -1; // MikanStageID
	int m_selectedSceneId = -1; // MikanSceneID
	int m_selectedCompositorId = -1; // MikanCompositorID
	int m_selectedSceneObjectIndex = -1;
	Rml::Vector<RmlModel_SceneObject> m_sceneOutliner;

	static bool s_bHasRegisteredTypes;
};
