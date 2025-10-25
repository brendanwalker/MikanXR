#pragma once

#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "SinglecastDelegate.h"

class RmlModel_AnchorComponent;
using RmlModel_AnchorComponentPtr = std::shared_ptr<RmlModel_AnchorComponent>;

class RmlModel_CompositorComponent;
using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;

class RmlModel_StencilComponent;
using RmlModel_StencilComponentPtr = std::shared_ptr<RmlModel_StencilComponent>;

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

	void selectStageEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
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

	void setSelectedStageId(int stageId);
	void setSelectedSceneId(int sceneId);

	void stageIdListChanged(bool bOwnerChanged);
	void sceneIdListChanged(bool bOwnerChanged);
	void compositorIdListChanged(bool bOwnerChanged);

	AnchorObjectSystemWeakPtr m_anchorSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;
	EditorObjectSystemWeakPtr m_editorSystem;
	StageObjectSystemWeakPtr m_stageSystem;
	SceneObjectSystemWeakPtr m_sceneSystem;
	StencilObjectSystemWeakPtr m_stencilSystem;

	RmlDataBinding_ComponentIdListPtr m_stageIdList;
	RmlDataBinding_ComponentIdListPtr m_sceneIdList;
	RmlModel_AnchorComponentPtr m_selectedAnchorModel;
	RmlModel_StencilComponentPtr m_selectedBoxStencilModel;
	RmlModel_StencilComponentPtr m_selectedModelStencilModel;
	RmlModel_StencilComponentPtr m_selectedQuadStencilModel;
	RmlModel_SceneComponentPtr m_selectedSceneModel;

	int m_selectedStageId = -1; // MikanStageID
	int m_selectedSceneId = -1; // MikanSceneID
	int m_selectedSceneObjectIndex = -1;
	Rml::Vector<RmlModel_SceneObject> m_sceneOutliner;

	static bool s_bHasRegisteredTypes;
};
