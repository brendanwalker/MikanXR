#pragma once

#include "Shared/GuiPanel.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "CameraObjectSystem.h"
#include "LightSystemFwd.h"
#include "MikanTypeFwd.h"
#include "MkGuiStyle.h"
#include "ObjectSystemFwd.h"
#include "StageObjectSystem.h"

#include <memory>

class GuiPanel_ProjectStages : public GuiPanel
{
public:
	GuiPanel_ProjectStages(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	StageObjectSystemPtr getStageSystem() const;
	CameraObjectSystemPtr getCameraSystem() const;
	RGBSpotLightSystemPtr getSpotLightSystem() const;
	RGBPixelGridSystemPtr getPixelGridSystem() const;
	StageComponentPtr getSelectedStage() const;
	CameraComponentPtr getSelectedCamera() const;
	void setSelectedStageId(MikanStageID stageId);
	void setSelectedCameraId(MikanCameraID cameraId);
	void setSelectedLightId(MikanLightID lightId, bool isGrid);

	class ProjectGuiPanelContext* m_context = nullptr;
	StageObjectSystemWeakPtr m_stageSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;
	RGBSpotLightSystemWeakPtr m_spotLightSystem;
	RGBPixelGridSystemWeakPtr m_pixelGridSystem;
	int m_selectedStageId = INVALID_MIKAN_ID;
	int m_selectedCameraId = INVALID_MIKAN_ID;
	int m_selectedLightId = INVALID_MIKAN_ID;
	bool m_selectedLightIsGrid = false;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	std::unique_ptr<GuiDataSource_ComboBox> m_stageDataSource;
	std::unique_ptr<GuiDataSource_ComboBox> m_cameraDataSource;
};
