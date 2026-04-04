#pragma once

#include "Shared/GuiPanel.h"
#include "CameraObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include "StageObjectSystem.h"

class GuiPanel_ProjectStages : public GuiPanel
{
public:
	GuiPanel_ProjectStages() = default;

	bool init(class ProjectGuiPanelContext* context);
	virtual void render(float deltaSeconds) override;
	virtual void dispose() override;

private:
	StageObjectSystemPtr getStageSystem() const;
	CameraObjectSystemPtr getCameraSystem() const;
	StageComponentPtr getSelectedStage() const;
	CameraComponentPtr getSelectedCamera() const;
	void setSelectedStageId(MikanStageID stageId);
	void setSelectedCameraId(MikanCameraID cameraId);

	class ProjectGuiPanelContext* m_context = nullptr;
	StageObjectSystemWeakPtr m_stageSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;
	int m_selectedStageId = INVALID_MIKAN_ID;
	int m_selectedCameraId = INVALID_MIKAN_ID;
};
