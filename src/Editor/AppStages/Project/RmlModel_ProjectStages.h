#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_ComponentList.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "SinglecastDelegate.h"

class RmlModel_CameraComponent;
using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;

class RmlModel_StageComponent;
using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;

class RmlModel_CompositorComponent;
using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;

class RmlModel_ProjectStages : public RmlModel
{
public:
	RmlModel_ProjectStages();

	bool init(
		Rml::Context* rmlContext, 
		ProjectConfigPtr projectConfig,
		StageObjectSystemPtr stageSystem,
		CameraObjectSystemPtr cameraSystem,
		CompositorObjectSystemPtr compositorSystem);
	virtual void dispose() override;

private:
	StageObjectSystemPtr getStageSystem();
	CameraObjectSystemPtr getCameraSystem();
	CompositorObjectSystemPtr getCompositorSystem();
	StageComponentPtr getSelectedStage();
	CameraComponentPtr getSelectedCamera();
	CompositorComponentPtr getSelectedCompositor();

	void addNewStage(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeStage(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewCamera(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeCamera(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewCompositor(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeCompositor(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectStageEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectCameraEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectCompositorEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedStageId(MikanStageID stageId);
	void setSelectedCameraId(MikanCameraID cameraId);
	void setSelectedCompositorId(MikanCompositorID compositorId);

	void stageIdListChanged(bool bOwnerChanged);
	void cameraIdListChanged(bool bOwnerChanged);
	void compositorIdListChanged(bool bOwnerChanged);

	ProjectConfigWeakPtr m_projectConfig;
	StageObjectSystemWeakPtr m_stageSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;
	CompositorObjectSystemWeakPtr m_compositorSystem;

	RmlDataBinding_ComponentListPtr m_stageIdList;
	RmlDataBinding_ComponentListPtr m_cameraIdList;
	RmlDataBinding_ComponentListPtr m_compositorIdList;
	RmlModel_StageComponentPtr m_selectedStageModel;
	RmlModel_CameraComponentPtr m_selectedCameraModel;
	RmlModel_CompositorComponentPtr m_selectedCompositorModel;

	int m_selectedStageId = -1; // MikanStageID
	int m_selectedCameraId = -1; // MikanCameraID
	int m_selectedCompositorId = -1; // MikanCompositorID
};
