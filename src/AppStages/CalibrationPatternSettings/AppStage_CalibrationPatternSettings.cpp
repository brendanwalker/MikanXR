//-- inludes -----
#include "MainMenu/AppStage_MainMenu.h"
#include "CalibrationPatternSettings/AppStage_CalibrationPatternSettings.h"
#include "App.h"
#include "ProfileConfig.h"
#include "Logger.h"
#include "MathUtility.h"
#include "MainWindow.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>


struct CalibrationPatternSettingsDataModel
{
	Rml::DataModelHandle model_handle;

	int selected_pattern= 0;

	float puck_horiz_offset = 75.f;
	float puck_vert_offset = 89.f;
	float puck_depth_offset = 0.f;

	void init(ProfileConfigConstPtr profileConfig)
	{
		selected_pattern = (int)profileConfig->calibrationPatternType;

		puck_horiz_offset = profileConfig->puckHorizontalOffsetMM;
		puck_vert_offset = profileConfig->puckVerticalOffsetMM;
		puck_depth_offset = profileConfig->puckDepthOffsetMM;
	}
};

//-- statics ----
const char* AppStage_CalibrationPatternSettings::APP_STAGE_NAME = "CalibrationPatternSettings";

//-- public methods -----
AppStage_CalibrationPatternSettings::AppStage_CalibrationPatternSettings(MainWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_CalibrationPatternSettings::APP_STAGE_NAME)
	, m_dataModel(new CalibrationPatternSettingsDataModel)
{
}

void AppStage_CalibrationPatternSettings::enter()
{
	AppStage::enter();
	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("calibration_pattern_settings");
	if (!constructor)
		return;

	m_dataModel->init(profileConfig);

	constructor.Bind("selected_pattern", &m_dataModel->selected_pattern);

	constructor.Bind("puck_horiz_offset", &m_dataModel->puck_horiz_offset);
	constructor.Bind("puck_vert_offset", &m_dataModel->puck_vert_offset);
	constructor.Bind("puck_depth_offset", &m_dataModel->puck_depth_offset);

	m_dataModel->model_handle = constructor.GetModelHandle();

	addRmlDocument("calibration_pattern_settings.rml");
}

void AppStage_CalibrationPatternSettings::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("calibration_pattern_settings");

	AppStage::exit();
}

void AppStage_CalibrationPatternSettings::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	bool bDirty= false;

	if (m_dataModel->model_handle.IsVariableDirty("selected_pattern"))
	{
		profileConfig->calibrationPatternType= (eCalibrationPatternType)m_dataModel->selected_pattern;
		bDirty= true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("puck_horiz_offset"))
	{
		profileConfig->puckHorizontalOffsetMM = m_dataModel->puck_horiz_offset;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("puck_vert_offset"))
	{
		profileConfig->puckVerticalOffsetMM = m_dataModel->puck_vert_offset;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("puck_depth_offset"))
	{
		profileConfig->puckDepthOffsetMM = m_dataModel->puck_depth_offset;
		bDirty = true;
	}

	if (bDirty)
	{
		profileConfig->save();
	}
}

void AppStage_CalibrationPatternSettings::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_main_menu")
	{
		m_ownerWindow->popAppState();
	}
}