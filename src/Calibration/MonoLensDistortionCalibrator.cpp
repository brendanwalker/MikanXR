#include "CalibrationRenderHelpers.h"
#include "CalibrationPatternFinder.h"
#include "Colors.h"
#include "GlCommon.h"
#include "Logger.h"
#include "MathUtility.h"
#include "MonoLensDistortionCalibrator.h"
#include "MathTypeConversion.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include <algorithm>
#include <atomic>
#include <thread>

struct MonoLensDistortionCalibrationState
{
	enum AsyncComputeStatus
	{
		NotStarted,
		Running,
		Succeded,
		Failed
	};

	// Static Input
	MikanMonoIntrinsics inputCameraIntrinsics;
	int desiredPatternCount;

	// Capture State
	int capturedPatternCount;
	t_opencv_point2d_list quadList;
	std::vector<t_opencv_point2d_list> imagePointsList;
	std::vector<t_opencv_pointID_list> imagePointIDList;

	// Async Calibration Compute
	std::thread* asyncComputeTask;
	std::atomic_int asyncComputeTaskStatus;

	// Result
	MikanMonoIntrinsics outputCameraIntrinsics;
	double reprojectionError;

	void init(VideoSourceViewPtr videoSourceView, int patternCount)
	{
		// Get the current camera intrinsics being used by the video source
		MikanVideoSourceIntrinsics cameraIntrinsics;
		videoSourceView->getCameraIntrinsics(cameraIntrinsics);
		assert(cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();
		desiredPatternCount = patternCount;

		resetCalibration();
	}

	void resetCalibration()
	{
		// Reset the capture state
		capturedPatternCount= 0;
		quadList.clear();
		imagePointsList.clear();

		// Reset the async task state
		asyncComputeTask = nullptr;
		asyncComputeTaskStatus = NotStarted;

		// Reset the output
		outputCameraIntrinsics = inputCameraIntrinsics;
		reprojectionError = 0.0;
	}
};

//-- MonoDistortionCalibrator ----
MonoLensDistortionCalibrator::MonoLensDistortionCalibrator(
	ProfileConfigConstPtr profileConfig,
	VideoFrameDistortionView* distortionView,
	int desiredBoardCount)
	: m_calibrationState(new MonoLensDistortionCalibrationState)
	, m_distortionView(distortionView)
	, m_patternFinder(CalibrationPatternFinder::allocatePatternFinder(profileConfig, distortionView))
{
	frameWidth = distortionView->getFrameWidth();
	frameHeight = distortionView->getFrameHeight();

	m_calibrationState->init(distortionView->getVideoSourceView(), desiredBoardCount);
}

MonoLensDistortionCalibrator::~MonoLensDistortionCalibrator()
{
	delete m_patternFinder;
}

void MonoLensDistortionCalibrator::findNewCalibrationPattern(const float minSeperationDist)
{
	m_patternFinder->findNewCalibrationPattern(minSeperationDist);
}

void MonoLensDistortionCalibrator::captureLastFoundCalibrationPattern()
{
	t_opencv_point2d_list imagePoints;
	t_opencv_pointID_list imagePointIDs;
	cv::Point2f boundingQuad[4];
	if (m_patternFinder->fetchLastFoundCalibrationPattern(imagePoints, imagePointIDs, boundingQuad))
	{
		m_calibrationState->imagePointsList.push_back(imagePoints);
		m_calibrationState->imagePointIDList.push_back(imagePointIDs);

		for (int i = 0; i < 4; ++i)
		{
			m_calibrationState->quadList.push_back(boundingQuad[i]);
		}

		++m_calibrationState->capturedPatternCount;
	}
}

bool MonoLensDistortionCalibrator::hasSampledAllCalibrationPatterns() const
{
	return m_calibrationState->capturedPatternCount >= m_calibrationState->desiredPatternCount;
}

bool MonoLensDistortionCalibrator::areCurrentImagePointsValid() const
{
	return m_patternFinder->areCurrentImagePointsValid();
}

float MonoLensDistortionCalibrator::computeCalibrationProgress() const
{
	const float samplePercentage =
		(float)m_calibrationState->capturedPatternCount
		/ (float)m_calibrationState->desiredPatternCount;

	return samplePercentage;
}

void MonoLensDistortionCalibrator::resetCalibrationState()
{
	m_calibrationState->resetCalibration();
}

void MonoLensDistortionCalibrator::resetDistortionView()
{
	m_distortionView->rebuildDistortionMap(&m_calibrationState->inputCameraIntrinsics);
}

void MonoLensDistortionCalibrator::computeCameraCalibration()
{
	assert(m_calibrationState->asyncComputeTask == nullptr);

	CalibrationPatternFinder* patternFinder = m_patternFinder;
	MonoLensDistortionCalibrationState* calibrationState= m_calibrationState;

	// Spin up a worker thread to compute the camera calibration.
	// The result of this is polled by getCameraCalibration().
	calibrationState->asyncComputeTaskStatus = MonoLensDistortionCalibrationState::Running;
	calibrationState->asyncComputeTask = new std::thread([patternFinder, calibrationState]
	{
		bool bSuccess= patternFinder->calibrateCamera(
			calibrationState->inputCameraIntrinsics,
			calibrationState->imagePointsList,
			calibrationState->imagePointIDList,
			calibrationState->outputCameraIntrinsics,
			calibrationState->reprojectionError);

		// Signal the main thread that the task is complete
		calibrationState->asyncComputeTaskStatus = 
			bSuccess 
			? MonoLensDistortionCalibrationState::Succeded
			: MonoLensDistortionCalibrationState::Failed;
	});
}

bool MonoLensDistortionCalibrator::getIsCameraCalibrationComplete()
{
	auto status= m_calibrationState->asyncComputeTaskStatus.load();
	
	return 
		status == MonoLensDistortionCalibrationState::Succeded ||
		status == MonoLensDistortionCalibrationState::Failed;
}

bool MonoLensDistortionCalibrator::getCameraCalibration(MikanMonoIntrinsics* out_mono_intrinsics)
{
	bool bFetchSuccess = false;

	if (m_calibrationState->asyncComputeTaskStatus.load() == MonoLensDistortionCalibrationState::Succeded)
	{
		if (m_calibrationState->asyncComputeTask != nullptr)
		{
			m_calibrationState->asyncComputeTask->join();

			delete m_calibrationState->asyncComputeTask;
			m_calibrationState->asyncComputeTask = nullptr;
		}

		*out_mono_intrinsics = m_calibrationState->outputCameraIntrinsics;
		bFetchSuccess = true;
	}

	return bFetchSuccess;
}

float MonoLensDistortionCalibrator::getReprojectionError() const
{
	return m_calibrationState->reprojectionError;
}

void MonoLensDistortionCalibrator::renderCalibrationState()
{
	// Draw the most recently capture chessboard
	m_patternFinder->renderCalibrationPattern2D();

	// Draw the outlines of all of the chess boards 
	if (m_calibrationState->quadList.size() > 0)
	{
		drawQuadList2d(
			frameWidth, frameHeight,
			(float*)m_calibrationState->quadList.data(), // cv::point2f is just two floats 
			(int)m_calibrationState->quadList.size(),
			Colors::Yellow);
	}
}