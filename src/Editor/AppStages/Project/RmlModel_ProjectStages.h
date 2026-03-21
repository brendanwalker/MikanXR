#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "SinglecastDelegate.h"

class RmlModel_CameraComponent;
using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;

class RmlModel_StageComponent;
using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;

class RmlModel_ProjectStages : public RmlModel
{
public:
	RmlModel_ProjectStages();

	bool init(class ProjectRmlModelContext* context);
	virtual void dispose() override;

private:
	StageObjectSystemPtr getStageSystem();
	CameraObjectSystemPtr getCameraSystem();
	StageComponentPtr getSelectedStage();
	CameraComponentPtr getSelectedCamera();
	CompositorComponentPtr getSelectedCompositor();

	void addNewStage(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeStage(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewCamera(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeCamera(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectStageEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectCameraEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedStageId(MikanStageID stageId);
	void setSelectedCameraId(MikanCameraID cameraId);

	void stageIdListChanged(bool bOwnerChanged);
	void cameraIdListChanged(bool bOwnerChanged);

	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;
	StageObjectSystemWeakPtr m_stageSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;

	RmlDataBinding_ComponentIdListPtr m_stageIdList;
	RmlDataBinding_ComponentIdListPtr m_cameraIdList;
	
	int m_selectedStageId = -1; // MikanStageID
	int m_selectedCameraId = -1; // MikanCameraID
};
