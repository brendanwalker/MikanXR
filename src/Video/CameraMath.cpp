#include "CameraMath.h"
#include "DeviceInterface.h"
#include "MikanMathTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "VideoSourceView.h"
#include "VRDeviceView.h"

#include "opencv2/opencv.hpp"

#define DEFAULT_MONO_HFOV   60.0   // 60 degrees
#define DEFAULT_MONO_ZNEAR  0.1    // 10cm
#define DEFAULT_MONO_ZFAR   20.0   // 20m

glm::mat4 computeGLMCameraViewMatrix(const glm::mat4& poseXform)
{
	// Convert the camera pose transform into a modelview matrix
	// This is adapted from glm::lookAt
	const glm::vec3 right = glm::vec3(poseXform[0]);
	const glm::vec3 up = glm::vec3(poseXform[1]);
	const glm::vec3 forward = glm::vec3(poseXform[2]);
	const glm::vec3 eye = glm::vec3(poseXform[3]);

	glm::mat4 modelView(1.f);
	modelView[0][0] = right.x;
	modelView[1][0] = right.y;
	modelView[2][0] = right.z;
	modelView[0][1] = up.x;
	modelView[1][1] = up.y;
	modelView[2][1] = up.z;
	modelView[0][2] = forward.x;
	modelView[1][2] = forward.y;
	modelView[2][2] = forward.z;
	modelView[3][0] = -glm::dot(right, eye);
	modelView[3][1] = -glm::dot(up, eye);
	modelView[3][2] = -glm::dot(forward, eye);

    return modelView;
}

void computeOpenCVCameraExtrinsicMatrix(
    VideoSourceViewPtr videoSource, 
    VRDeviceViewPtr trackingPuck,
    cv::Matx34f &out)
{
    // Extrinsic matrix is the inverse of the camera pose matrix
    const glm::mat4 glm_camera_xform = videoSource->getCameraPose(trackingPuck);
    const glm::mat4 glm_mat = glm::inverse(glm_camera_xform);

    out(0, 0) = glm_mat[0][0]; out(0, 1) = glm_mat[1][0]; out(0, 2) = glm_mat[2][0]; out(0, 3) = glm_mat[3][0];
    out(1, 0) = glm_mat[0][1]; out(1, 1) = glm_mat[1][1]; out(1, 2) = glm_mat[2][1]; out(1, 3) = glm_mat[3][1];
    out(2, 0) = glm_mat[0][2]; out(2, 1) = glm_mat[1][2]; out(2, 2) = glm_mat[2][2]; out(2, 3) = glm_mat[3][2];
}

bool computeOpenCVCameraRelativePatternTransform(
	const MikanMonoIntrinsics& intrinsics,
    const t_opencv_point2d_list& imagePoints,
    const t_opencv_point3d_list& objectPointsMM,
    cv::Quatd& outOrientation,
    cv::Vec3d& outPositionMM)
{
    // Bail if there isn't a corresponding world point for every image point
	if (imagePoints.size() != objectPointsMM.size() || imagePoints.size() == 0)
	{
		return false;
	}

	// Fetch the 3x3 GLM camera intrinsic matrix and store into a 3x3 openCV matrix
	const MikanMatrix3d& glmIntrinsicMatrix = intrinsics.camera_matrix;
	cv::Matx33d cvIntrinsicMatrix = MikanMatrix3d_to_cv_mat33d(glmIntrinsicMatrix);

	// Store the distortion parameters in a row vector with 8 values: [k1, k2, p1, p2, k3, k4, k5, k6]
	const MikanDistortionCoefficients& distortion_coeffs = intrinsics.distortion_coefficients;
	cv::Matx81d cvDistCoeffsColVector = Mikan_distortion_to_cv_vec8(distortion_coeffs);
	cv::Mat cvDistCoeffsRowVector;
	cv::transpose(cvDistCoeffsColVector, cvDistCoeffsRowVector);

    // Use AP3P solver if we have exactly 4 points, otherwise use the iterative solver
    // since the iterative solver needs at least 6 points
	int solver = imagePoints.size() == 4 ? cv::SOLVEPNP_AP3P : cv::SOLVEPNP_ITERATIVE;

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the mat relative to the camera
	cv::Mat rvec;
	cv::Mat tvecMM; // Mat position in millimeters
	if (!cv::solvePnP(
		objectPointsMM, imagePoints,
		cvIntrinsicMatrix, cvDistCoeffsRowVector,
		rvec, tvecMM, 
        false, // useExtrinsicGuess
        solver))
	{
		return false;
	}

    outOrientation = cv::Quatd::createFromRvec(rvec);
    outPositionMM= tvecMM;

	return true;
}

void convertOpenCVCameraRelativePoseToGLMMat(
    const cv::Quatd& orientation,
    const cv::Vec3d& positionMM,
    glm::dmat4& outXform)
{
    // OpenCV camera relative locations in millimeters (by convention from object points passed into solvePnP), 
    // but the output GLM transform is in meters (by convention as defined by our renderer)
    cv::Vec3d tvec = positionMM * k_millimeters_to_meters;

	// Convert the OpenCV Rodrigues vector used to store orientation into a 3x3 rotation matrix
    cv::Matx33d rmat = orientation.toRotMat3x3();

	// Compose the openCV  rotation and translation together,
	// then convert OpenCV coordinates to OpenGL coordinates 
    // (x, y, z) -> (x, -y, -z)
	cv::Matx44d cv_RTMat = cv::Matx44d(
		rmat(0, 0), rmat(0, 1), rmat(0, 2), tvec(0),
		-rmat(1, 0), -rmat(1, 1), -rmat(1, 2), -tvec(1),
		-rmat(2, 0), -rmat(2, 1), -rmat(2, 2), -tvec(2),
		0.0F, 0.0F, 0.0F, 1.0F
	);

	// Convert OpenCV matrix to GLM matrix
    glm::mat4 RTMat;
	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			// GLM indexed by column first
			// OpenCV indexed by row first
            RTMat[col][row] = cv_RTMat(row, col);
		}
	}

	// Optionally, rotate result about the x-axis by specified amount.
	glm::mat4 flipAboutX = glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	outXform = RTMat * flipAboutX;
}

void computeOpenCVCameraIntrinsicMatrix(
    VideoSourceViewPtr videoSource,
    VideoFrameSection section,
    cv::Matx33f& intrinsicOut)
{
    MikanVideoSourceIntrinsics tracker_intrinsics;
    videoSource->getCameraIntrinsics(tracker_intrinsics);

    MikanMatrix3d camera_matrix;
    bool validCameraMatrix= false;

    if (tracker_intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
    {
        if (section == VideoFrameSection::Left)
        {
            camera_matrix= tracker_intrinsics.getStereoIntrinsics().left_camera_matrix;
            validCameraMatrix= true;
        }
        else if (section == VideoFrameSection::Right)
        {
            camera_matrix= tracker_intrinsics.getStereoIntrinsics().right_camera_matrix;
            validCameraMatrix= true;
        }
    }
    else if (tracker_intrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS)
    {
        camera_matrix = tracker_intrinsics.getMonoIntrinsics().camera_matrix;
        validCameraMatrix= true;
    }

    if (validCameraMatrix)
    {  
        intrinsicOut= MikanMatrix3d_to_cv_mat33f(camera_matrix);
    }
}

void extractCameraIntrinsicMatrixParameters(
    const MikanMatrix3d& intrinsic_matrix,
    float& out_focal_length_x,
    float& out_focal_length_y,
    float& out_principal_point_x,
    float& out_principal_point_y)
{
	out_focal_length_x = intrinsic_matrix.x0;
	out_focal_length_y = intrinsic_matrix.y1;
	out_principal_point_x = intrinsic_matrix.z0;
	out_principal_point_y = intrinsic_matrix.z1;
}

void extractCameraIntrinsicMatrixParameters(
    const cv::Matx33f &intrinsic_matrix,
    float &out_focal_length_x,
    float &out_focal_length_y,
    float &out_principal_point_x,
    float &out_principal_point_y)
{
	out_focal_length_x= intrinsic_matrix(0, 0);
	out_focal_length_y= intrinsic_matrix(1, 1);
	out_principal_point_x= intrinsic_matrix(0, 2);
	out_principal_point_y= intrinsic_matrix(1, 2);
}

bool computeOpenCVCameraRectification(
    VideoSourceViewPtr videoSource,
    VideoFrameSection section,
    cv::Matx33d &rotationOut,
    cv::Matx34d &projectionOut)
{
    MikanVideoSourceIntrinsics tracker_intrinsics;
    videoSource->getCameraIntrinsics(tracker_intrinsics);

    MikanMatrix3d rectification_rotation;
    MikanMatrix4x3d rectification_projection;
    bool validRectification= false;

    if (tracker_intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
    {
		const auto& stereoIntrinsics = tracker_intrinsics.getStereoIntrinsics();

        if (section == VideoFrameSection::Left)
        {
            rectification_rotation= stereoIntrinsics.left_rectification_rotation;
            rectification_projection= stereoIntrinsics.left_rectification_projection;
            validRectification= true;
        }
        else if (section == VideoFrameSection::Right)
        {
            rectification_rotation= stereoIntrinsics.right_rectification_rotation;
            rectification_projection= stereoIntrinsics.right_rectification_projection;
            validRectification= true;
        }
    }

    if (validRectification)
    {
        rotationOut = MikanMatrix3d_to_cv_mat33d(rectification_rotation);
        projectionOut = MikanMatrix4x3d_to_cv_mat34d(rectification_projection);

        return true;
    }
    else
    {
        return false;
    }
}

void createDefautMonoIntrinsics(
    int pixelWidth, 
    int pixelHeight,
    MikanMonoIntrinsics &outIntrinsics)
{
    outIntrinsics = MikanMonoIntrinsics();

    double aspectRatio= (double)pixelHeight / (double)pixelWidth;
    double c_x= (double)pixelWidth / 2.0;
    double c_y= (double)pixelHeight / 2.0;
    double hfov= DEFAULT_MONO_HFOV;
    double vfov= hfov * aspectRatio;
    double f_x= c_x / tan(glm::radians(hfov / 2.0));
    double f_y= c_y / tan(glm::radians(vfov / 2.0));

	outIntrinsics.pixel_width= (double)pixelWidth;
	outIntrinsics.pixel_height= (double)pixelHeight;
	outIntrinsics.hfov= hfov;
	outIntrinsics.vfov= vfov;
	outIntrinsics.znear= DEFAULT_MONO_ZNEAR;
	outIntrinsics.zfar= DEFAULT_MONO_ZFAR;	
	outIntrinsics.camera_matrix = {
		f_x, 0.0, c_x,
		0.0, f_y, c_y,
		0.0, 0.0, 1.0
	};
}
