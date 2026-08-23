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
	GuiPanel_ProjectStages(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	StageObjectSystemPtr getStageSystem() const;
	LightEnvironmentSystemPtr getLightEnvironmentSystem() const;
	CameraObjectSystemPtr getCameraSystem() const;
	RGBSpotLightSystemPtr getSpotLightSystem() const;
	RGBPixelGridSystemPtr getPixelGridSystem() const;
	StageComponentPtr getSelectedStage() const;
	CameraComponentPtr getSelectedCamera() const;
	void setSelectedStageId(MikanStageID stageId);
	void setSelectedCameraId(MikanCameraID cameraId);
	/// Which system the selected light belongs to. Was a bool while there were
	/// only two kinds; environment probes made that a third case.
	enum class eSelectedLightKind : int
	{
		none,
		spot,
		pixelGrid,
		environment,
	};

	void setSelectedLightId(MikanLightID lightId, eSelectedLightKind lightKind);

	class ProjectGuiPanelContext* m_context= nullptr;
	StageObjectSystemWeakPtr m_stageSystem;
	CameraObjectSystemWeakPtr m_cameraSystem;
	LightEnvironmentSystemWeakPtr m_lightEnvironmentSystem;
	RGBSpotLightSystemWeakPtr m_spotLightSystem;
	RGBPixelGridSystemWeakPtr m_pixelGridLightSystem;
	int m_selectedStageId= INVALID_MIKAN_ID;
	int m_selectedCameraId= INVALID_MIKAN_ID;
	int m_selectedLightId= INVALID_MIKAN_ID;
	eSelectedLightKind m_selectedLightKind= eSelectedLightKind::none;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	std::unique_ptr<GuiDataSource_ComboBox> m_stageDataSource;
	std::unique_ptr<GuiDataSource_ComboBox> m_cameraDataSource;
};
