//-- inludes -----
#include "MainMenu/AppStage_MainMenu.h"
#include "CalibrationPatternSettings/AppStage_CalibrationPatternSettings.h"
#include "App.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "Logger.h"
#include "MathUtility.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>


struct CalibrationPatternSettingsDataModel
{
	Rml::DataModelHandle model_handle;

	int selected_pattern= 0;

	int chessboard_rows = 5;
	int chessboard_cols = 5;
	float square_length = 30.f;

	int circle_grid_rows = 11;
	int circle_grid_cols = 4;
	float circle_spacing = 20.f;
	float circle_diameter = 15.f;

	float puck_horiz_offset = 75.f;
	float puck_vert_offset = 89.f;
	float puck_depth_offset = 0.f;

	void init(const ProfileConfig* profileConfig)
	{
		selected_pattern = (int)profileConfig->calibrationPatternType;

		chessboard_rows = profileConfig->chessbordRows;
		chessboard_cols = profileConfig->chessbordCols;
		square_length = profileConfig->squareLengthMM;

		circle_grid_rows = profileConfig->circleGridRows;
		circle_grid_cols = profileConfig->circleGridCols;
		circle_spacing = profileConfig->circleSpacingMM;
		circle_diameter = profileConfig->circleDiameterMM;

		puck_horiz_offset = profileConfig->puckHorizontalOffsetMM;
		puck_vert_offset = profileConfig->puckVerticalOffsetMM;
		puck_depth_offset = profileConfig->puckDepthOffsetMM;
	}
};

//-- statics ----
const char* AppStage_CalibrationPatternSettings::APP_STAGE_NAME = "CalibrationPatternSettings";

//-- public methods -----
AppStage_CalibrationPatternSettings::AppStage_CalibrationPatternSettings(App* app)
	: AppStage(app, AppStage_CalibrationPatternSettings::APP_STAGE_NAME)
	, m_dataModel(new CalibrationPatternSettingsDataModel)
{
}

void AppStage_CalibrationPatternSettings::enter()
{
	AppStage::enter();
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	Rml::DataModelConstructor constructor = getRmlContext()->CreateDataModel("calibration_pattern_settings");
	if (!constructor)
		return;

	m_dataModel->init(profileConfig);

	constructor.Bind("selected_pattern", &m_dataModel->selected_pattern);
	constructor.Bind("chessboard_rows", &m_dataModel->chessboard_rows);
	constructor.Bind("chessboard_cols", &m_dataModel->chessboard_cols);
	constructor.Bind("square_length", &m_dataModel->square_length);
	constructor.Bind("circle_grid_rows", &m_dataModel->circle_grid_rows);
	constructor.Bind("circle_grid_cols", &m_dataModel->circle_grid_cols);
	constructor.Bind("circle_spacing", &m_dataModel->circle_spacing);
	constructor.Bind("circle_diameter", &m_dataModel->circle_diameter);
	constructor.Bind("puck_horiz_offset", &m_dataModel->puck_horiz_offset);
	constructor.Bind("puck_vert_offset", &m_dataModel->puck_vert_offset);
	constructor.Bind("puck_depth_offset", &m_dataModel->puck_depth_offset);

	m_dataModel->model_handle = constructor.GetModelHandle();

	addRmlDocument("rml\\calibration_pattern_settings.rml");
}

void AppStage_CalibrationPatternSettings::exit()
{
	// Clean up the data model
	getRmlContext()->RemoveDataModel("calibration_pattern_settings");

	AppStage::exit();
}

void AppStage_CalibrationPatternSettings::update()
{
	ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	bool bDirty= false;

	if (m_dataModel->model_handle.IsVariableDirty("selected_pattern"))
	{
		profileConfig->calibrationPatternType= (eCalibrationPatternType)m_dataModel->selected_pattern;
		bDirty= true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("chessboard_rows"))
	{
		profileConfig->chessbordRows = m_dataModel->chessboard_rows;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("chessboard_cols"))
	{
		profileConfig->chessbordCols = m_dataModel->chessboard_cols;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("square_length"))
	{
		profileConfig->squareLengthMM = m_dataModel->square_length;
		bDirty = true;
	}

	if (m_dataModel->model_handle.IsVariableDirty("chessboard_rows"))
	{
		profileConfig->chessbordRows = m_dataModel->chessboard_rows;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("circle_grid_cols"))
	{
		profileConfig->circleGridCols = m_dataModel->circle_grid_cols;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("circle_spacing"))
	{
		profileConfig->circleSpacingMM = m_dataModel->circle_spacing;
		bDirty = true;
	}
	if (m_dataModel->model_handle.IsVariableDirty("circle_diameter"))
	{
		profileConfig->circleDiameterMM = m_dataModel->circle_diameter;
		bDirty = true;
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
		m_app->popAppState();
	}
}