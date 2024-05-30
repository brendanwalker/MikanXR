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
	OpenCVCalibrationGeometry inputObjectGeometry;
	int desiredPatternCount;

	// Capture State
	int capturedPatternCount;
	t_opencv_point2d_list quadList;
	std::vector<t_opencv_point2d_list> imagePointsList;

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

		inputCameraIntrinsics= cameraIntrinsics.intrinsics.mono;
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
	cv::Point2f boundingQuad[4];
	if (m_patternFinder->fetchLastFoundCalibrationPattern(imagePoints, boundingQuad))
	{
		m_calibrationState->imagePointsList.push_back(imagePoints);

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

	MonoLensDistortionCalibrationState* calibrationState= m_calibrationState;

	// Cache the 3d geometry of the calibration pattern in the calibration state
	m_patternFinder->getOpenCVLensCalibrationGeometry(&calibrationState->inputObjectGeometry);

	// Spin up a worker thread to compute the camera calibration.
	// The result of this is polled by getCameraCalibration().
	calibrationState->asyncComputeTaskStatus = MonoLensDistortionCalibrationState::Running;
	calibrationState->asyncComputeTask = new std::thread([calibrationState]
	{
		const int frameWidth= (int)calibrationState->inputCameraIntrinsics.pixel_width;
		const int frameHeight= (int)calibrationState->inputCameraIntrinsics.pixel_height;

		// We maintain some properties of the existing intrinsic matrix 
		// so we need to use the current intrinsics as input (see options below)
		const MikanMatrix3d& mikanIntrinsicMatrix= calibrationState->inputCameraIntrinsics.camera_matrix;
		cv::Matx33d cvIntrinsicMatrix = MikanMatrix3d_to_cv_mat33d(mikanIntrinsicMatrix);

		// Get the image point sets we captured during calibration
		const auto& cvImagePointsList= calibrationState->imagePointsList;
		const size_t imagePointSetCount = cvImagePointsList.size();

		// Each 2d image point set should have a corresponding 3d object point set
		std::vector< t_opencv_point3d_list > cvObjectPointsList(imagePointSetCount);
		std::fill(cvObjectPointsList.begin(), cvObjectPointsList.end(), calibrationState->inputObjectGeometry.points);

		// Compute the camera intrinsic matrix and distortion parameters
		cv::Mat cvDistCoeffsRowVector;
		double reprojectionError = 0.0;
		bool bSuccess= true;
		try
		{
			reprojectionError =
				cv::calibrateCamera(
					cvObjectPointsList,
					cvImagePointsList,
					cv::Size(frameWidth, frameHeight),
					cvIntrinsicMatrix, // Input/Output camera intrinsic matrix 
					cvDistCoeffsRowVector, // Output distortion coefficients
					cv::noArray(), cv::noArray(), // best fit board poses as rvec/tvec pairs
					cv::CALIB_FIX_ASPECT_RATIO + // The functions considers only fy as a free parameter
					cv::CALIB_FIX_PRINCIPAL_POINT + // The principal point is not changed during the global optimization
					cv::CALIB_ZERO_TANGENT_DIST + // Tangential distortion coefficients (p1,p2) are set to zeros and stay zero
					cv::CALIB_RATIONAL_MODEL + // Coefficients k4, k5, and k6 are enabled
					cv::CALIB_FIX_K3 + cv::CALIB_FIX_K4 + cv::CALIB_FIX_K5, // radial distortion coefficients k3, k4, & k5 are not changed during the optimization
					cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, DBL_EPSILON));

			bSuccess= is_valid_float(reprojectionError);
		}
		catch (cv::Exception* e)
		{
			MIKAN_MT_LOG_ERROR("computeCameraCalibration") << "Error computing lens calibration: " << e->msg;
			bSuccess= false;
		}

		if (bSuccess)
		{
			// cv::calibrateCamera() will return all 14 distortion parameters, but we only want the first 8
			cv::Mat cvDistCoeffsColVector;
			cv::transpose(cvDistCoeffsRowVector.colRange(cv::Range(0, 8)), cvDistCoeffsColVector);

			// Write the calibration output state
			calibrationState->reprojectionError = reprojectionError;
			calibrationState->outputCameraIntrinsics = calibrationState->inputCameraIntrinsics;
			calibrationState->outputCameraIntrinsics.camera_matrix = cv_mat33d_to_MikanMatrix3d(cvIntrinsicMatrix);
			calibrationState->outputCameraIntrinsics.distortion_coefficients = cv_vec8_to_Mikan_distortion(cvDistCoeffsColVector);

			// Derive the FoV angles from the image size and the newly computed intrinsic matrix
			double unusedFocalLength;
			cv::Point2d ununsedPrincipalPoint;
			double unusedAspectRatio;
			cv::calibrationMatrixValues(
				cvIntrinsicMatrix,
				cv::Size(frameWidth, frameHeight),
				0.0, 0.0, // Don't know (and don't need) the physical aperture size of the lens
				calibrationState->outputCameraIntrinsics.hfov,
				calibrationState->outputCameraIntrinsics.vfov,
				unusedFocalLength,
				ununsedPrincipalPoint,
				unusedAspectRatio);
		}

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