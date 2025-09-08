#include "CalibrationPatternFinder_Aruco.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "CameraMath.h"
#include "Colors.h"
#include "IEditorWindow.h"
#include "MathOpenCV.h"
#include "MathUtility.h"
#include "MathTypeConversion.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MikanTextRenderer.h"
#include "MikanObjectSystem.h"
#include "ObjectSystemManager.h"
#include "StageComponent.h"
#include "TextStyle.h"
#include "TrackingVolumeDefinition.h"
#include "VideoFrameDistortionView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/objdetect/aruco_detector.hpp"

//-- ArucoBoardData -----
class ArucoBoardData
{
public:
	ArucoBoardData() = default;

	int desiredArucoId;
	float markerLengthMM;

	cv::Ptr<cv::aruco::ArucoDetector> detector;
	std::vector<t_opencv_point2d_list> markerCorners;
	std::vector<int> markerVisibleIds;
	t_opencv_point2d_list charucoCorners;
	std::vector<int> charucoIds;
};

//-- CalibrationPatternFinder_Aruco -----
CalibrationPatternFinder_Aruco::CalibrationPatternFinder_Aruco(
	CameraComponentConstPtr cameraComponent,
	VideoFrameDistortionView* distortionView)
	: CalibrationPatternFinder(distortionView)
	, m_markerData(new ArucoBoardData())
{
	StageComponentConstPtr ownerStage= cameraComponent->getOwnerStageComponent();
	assert(ownerStage != nullptr);
	IEditorWindow* ownerWindow= static_cast<IEditorWindow *>(ownerStage->getOwnerWindow());
	assert(ownerWindow != nullptr);
	MarkerObjectSystemPtr markerSystem= 
		ownerWindow->getObjectSystemManager()->getSystemOfType<MarkerObjectSystem>();
	assert(markerSystem != nullptr);
	TrackingVolumeDefinitionConstPtr trackingSystem = ownerStage->getTrackingVolumeDefinitionConst();
	assert(trackingSystem != nullptr);
	MarkerDefinitionConstPtr originMarker= trackingSystem->getOriginMarker();
	assert(originMarker != nullptr);

	int desiredArucoId = originMarker->getArucoId();
	float markerLengthMM = originMarker->getLengthMM();
	eCharucoDictionaryType charucoDictionaryType = 
		markerSystem->getMarkerSystemConfig()->getArucoDictionaryType();

	cv::aruco::PredefinedDictionaryType cvCharucoDictionary = cv::aruco::DICT_6X6_250;
	switch (charucoDictionaryType)
	{
		case eCharucoDictionaryType::DICT_4X4:
			cvCharucoDictionary = cv::aruco::DICT_4X4_250;
			break;
		case eCharucoDictionaryType::DICT_5X5:
			cvCharucoDictionary = cv::aruco::DICT_5X5_250;
			break;
		case eCharucoDictionaryType::DICT_6X6:
			cvCharucoDictionary = cv::aruco::DICT_6X6_250;
			break;
		case eCharucoDictionaryType::DICT_7X7:
			cvCharucoDictionary = cv::aruco::DICT_7X7_250;
			break;
		default:
			break;
	}
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cvCharucoDictionary);

	// Use corner refinement to get the best possible corner locations
	cv::aruco::DetectorParameters detectorParams;
	detectorParams.cornerRefinementMethod= cv::aruco::CORNER_REFINE_SUBPIX;

	m_markerData->desiredArucoId = desiredArucoId;
	m_markerData->markerLengthMM = markerLengthMM;
	m_markerData->detector = cv::makePtr<cv::aruco::ArucoDetector>(dictionary, detectorParams);

	// The Aruco board is a square, so we can hardcode the points in ARUCO_CCW_CENTER style
	// Solve PnP points are on the XZ Plane
	m_opencvSolvePnPGeometry.points.clear();
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(-markerLengthMM / 2.f, 0.f, markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(markerLengthMM / 2.f, 0.f, markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(markerLengthMM / 2.f, 0.f, -markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(-markerLengthMM / 2.f, 0.f, -markerLengthMM / 2.f));

	// Derive the other geometry from the OpenCV SolvePnP geometry
	m_opencvLensCalibrationGeometry.points.clear();
	m_openglSolvePnPGeometry.points.clear();
	for (int index = 0; index < 4; index++)
	{
		// Solve PnP points are on the XZ Plane
		const cv::Point3f& openCVSolvePnPPoint= m_opencvSolvePnPGeometry.points[index];

		// Lens calibration points are on the XY Plane
		cv::Point3f openCVLensCalibrationPoint(
			openCVSolvePnPPoint.x,
			openCVSolvePnPPoint.z,
			0.f);
		m_opencvLensCalibrationGeometry.points.push_back(openCVLensCalibrationPoint);

		// OpenCV -> OpenGL coordinate system transform
		// Rendering world units in meters, not mm
		glm::vec3 openGLPoint(
			openCVSolvePnPPoint.x * k_millimeters_to_meters,
			-openCVSolvePnPPoint.y * k_millimeters_to_meters,
			-openCVSolvePnPPoint.z * k_millimeters_to_meters);
		m_openglSolvePnPGeometry.points.push_back(openGLPoint);
	}
}

CalibrationPatternFinder_Aruco::~CalibrationPatternFinder_Aruco()
{
	delete m_markerData;
}

bool CalibrationPatternFinder_Aruco::findNewCalibrationPattern(const float minSeperationDist)
{
	// Clear out the previous images points
	bool bImagePointsValid = false;
	m_currentImagePoints.clear();

	// Fetch the source image buffer we are searching for the pattern in
	cv::Mat* gsSourceBuffer = getGrayscaleVideoFrameInput();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find Arcuo marker corners on the small image
	m_markerData->markerCorners.clear();
	m_markerData->detector->detectMarkers(
		*gsSourceBuffer,
		m_markerData->markerCorners,
		m_markerData->markerVisibleIds);
	const bool bFoundMarkers = m_markerData->markerVisibleIds.size() > 0;

	// Re-clear out the image points if we decided the latest captured onces are invalid
	if (bFoundMarkers)
	{
		for (int index = 0; index < m_markerData->markerVisibleIds.size(); ++index)
		{
			if (m_markerData->markerVisibleIds[index] == m_markerData->desiredArucoId)
			{
				m_currentImagePoints= m_markerData->markerCorners[index];
				break;
			}
		}
	}
	else
	{
		m_currentImagePoints.clear();
	}

	return bFoundMarkers;
}

bool CalibrationPatternFinder_Aruco::fetchLastFoundCalibrationPattern(
	t_opencv_point2d_list& outImagePoints,
	t_opencv_pointID_list& outImagePointIDs,
	cv::Point2f outBoundingQuad[4])
{
	// If it's a valid new location, append it to the board list
	if (areCurrentImagePointsValid())
	{
		// Keep track of the corners of all of the chessboards we sample
		outBoundingQuad[0] = m_currentImagePoints[0];
		outBoundingQuad[1] = m_currentImagePoints[1];
		outBoundingQuad[2] = m_currentImagePoints[2];
		outBoundingQuad[3] = m_currentImagePoints[3];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
		}

		outImagePointIDs.clear();
		outImagePointIDs.push_back(m_markerData->desiredArucoId);

		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

void CalibrationPatternFinder_Aruco::renderCalibrationPattern2D() const
{
	CalibrationPatternFinder::renderCalibrationPattern2D();

	// Draw the marker corners, if any
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Middle;
	style.verticalAlignment = eVerticalTextAlignment::Middle;
	style.color = Colors::Yellow;

	static int debugDrawIndex = -1;

	for (int quadIndex = 0; quadIndex < m_markerData->markerCorners.size(); quadIndex++)
	{
		if (debugDrawIndex != -1 && debugDrawIndex != quadIndex)
			continue;

		const t_opencv_point2d_list& corners = m_markerData->markerCorners[quadIndex];

		drawQuadList2d(
			m_frameWidth, m_frameHeight,
			(float*)corners.data(), // cv::point2f is just two floats 
			(int)corners.size(),
			Colors::Yellow);

		if (quadIndex < m_markerData->markerVisibleIds.size())
		{
			int markerId = m_markerData->markerVisibleIds[quadIndex];

			cv::Point2f quadCenter;
			opencv_point2f_compute_average(corners, quadCenter);

			drawTextAtTrackerPosition(
				style,
				m_frameWidth, m_frameHeight,
				glm::vec2(quadCenter.x, quadCenter.y),
				L"%d", markerId);
		}
	}
}
