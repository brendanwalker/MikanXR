#include "NaturalFeatureCloudBuilder.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "CameraMath.h"
#include "Colors.h"
#include "MathGLM.h"
#include "MikanLineRenderer.h"
#include "MikanVideoSourceTypes.h"
#include "VideoFrameDistortionView.h"

#include <algorithm>
#include <cmath>

#include "opencv2/opencv.hpp"

// -- tuning constants -----
namespace
{
// Feature detection / tracking
const int k_targetFeatureCount= 400;
const double k_goodFeaturesQualityLevel= 0.01;
const double k_goodFeaturesMinDistancePx= 12.0;
const int k_lkWindowSize= 21;
const int k_lkPyramidLevels= 3;
const float k_maxLkErrorPx= 20.f;

// Keyframe gating (parallax comes from translation; rotation opens new viewpoints)
const float k_minKeyframeBaselineMeters= 0.03f; // 3 cm
const float k_minKeyframeAngleDeg= 5.f;
const int k_maxObservationsPerTrack= 8;

// Triangulation acceptance
const float k_minParallaxDeg= 2.0f;
const float k_maxReprojErrorPx= 2.0f;
const float k_maxTriangulationDistMeters= 10.0f;

// Usable-cloud thresholds
const int k_minUsableCloudPoints= 40;
const float k_minUsableCoverageMeters= 0.05f;
} // namespace

// -- internal state -----
struct FeatureTrack
{
	cv::Point2f lastPixel;
	bool triangulated= false;
	std::vector<glm::mat4> obsPoses;  // keyframe camera poses (stage space)
	std::vector<glm::vec2> obsPixels; // matching pixel observations
};

struct NaturalFeatureCloudState
{
	MikanMonoIntrinsics inputCameraIntrinsics;
	int frameWidth= 0;
	int frameHeight= 0;

	cv::Mat prevGray;
	std::vector<FeatureTrack> tracks;

	bool hasKeyframe= false;
	glm::mat4 lastKeyframePose= glm::mat4(1.f);
	int keyframeCount= 0;

	std::vector<NaturalFeaturePoint> cloud;

	bool hasRoi= false;
	glm::vec2 roiMin= glm::vec2(0.f);
	glm::vec2 roiMax= glm::vec2(0.f);

	void init(CameraComponentPtr cameraComponent, VideoFrameDistortionView* distortionView)
	{
		MikanVideoSourceIntrinsics cameraIntrinsics;
		cameraComponent->getApertureIntrinsics(cameraIntrinsics);
		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();

		frameWidth= distortionView->getFrameWidth();
		frameHeight= distortionView->getFrameHeight();

		reset();
	}

	void reset()
	{
		prevGray= cv::Mat();
		tracks.clear();
		hasKeyframe= false;
		lastKeyframePose= glm::mat4(1.f);
		keyframeCount= 0;
		cloud.clear();
	}
};

// -- helpers -----
namespace
{
bool isPixelInFrame(const cv::Point2f& p, int width, int height)
{
	return p.x >= 0.f && p.y >= 0.f && p.x < (float)width && p.y < (float)height;
}

bool isPixelInRoi(const NaturalFeatureCloudState* state, const cv::Point2f& p)
{
	if (!state->hasRoi)
		return true;

	return p.x >= state->roiMin.x && p.x <= state->roiMax.x && p.y >= state->roiMin.y && p.y <= state->roiMax.y;
}

// Project a world point into a camera's pixel space (inverse of computeCameraRayAtPixel).
// Returns false if the point is behind the camera (fails cheirality).
bool projectWorldPointToPixel(const MikanMonoIntrinsics& intrinsics, const glm::mat4& cameraXform,
							  const glm::vec3& worldPoint, glm::vec2& outPixel)
{
	const glm::vec3 right= cameraXform[0];
	const glm::vec3 up= cameraXform[1];
	const glm::vec3 forward= glm::vec3(cameraXform[2]) * -1.f; // -Z is forward
	const glm::vec3 eye= cameraXform[3];

	const glm::vec3 v= worldPoint - eye;
	const float depth= glm::dot(v, forward);
	if (depth <= 0.f)
		return false;

	float fx, fy, cx, cy, skew;
	extractCameraIntrinsicMatrixParameters(intrinsics.undistorted_camera_matrix, fx, fy, cx, cy, skew);

	const float localX= glm::dot(v, right) / depth;
	const float localY= glm::dot(v, up) / depth;

	outPixel.x= cx + localX * fx;
	outPixel.y= cy - localY * fy; // flip y (matches computeCameraRayAtPixel)

	return true;
}

// Triangulate the track's widest-baseline observation pair, validate reprojection across all views.
bool triangulateTrack(const MikanMonoIntrinsics& intrinsics, const FeatureTrack& track, NaturalFeaturePoint& outPoint)
{
	const size_t obsCount= track.obsPoses.size();
	if (obsCount < 2)
		return false;

	// Precompute back-projected rays for every observation
	std::vector<glm::vec3> rayStarts(obsCount);
	std::vector<glm::vec3> rayDirs(obsCount);
	for (size_t i= 0; i < obsCount; ++i)
	{
		computeCameraRayAtPixel(intrinsics, track.obsPoses[i], track.obsPixels[i], rayStarts[i], rayDirs[i]);
	}

	// Find the pair with the widest parallax angle
	float bestParallaxDeg= 0.f;
	size_t bestI= 0, bestJ= 1;
	for (size_t i= 0; i < obsCount; ++i)
	{
		for (size_t j= i + 1; j < obsCount; ++j)
		{
			const float cosAngle= glm::clamp(glm::dot(rayDirs[i], rayDirs[j]), -1.f, 1.f);
			const float angleDeg= glm::degrees(acosf(cosAngle));
			if (angleDeg > bestParallaxDeg)
			{
				bestParallaxDeg= angleDeg;
				bestI= i;
				bestJ= j;
			}
		}
	}

	if (bestParallaxDeg < k_minParallaxDeg)
		return false;

	// Triangulate the widest-baseline pair via ray-ray closest approach (AnchorTriangulator precedent)
	float closestTime= 0.f;
	glm::vec3 worldPoint;
	if (!glm_closest_point_on_ray_to_ray(rayStarts[bestI], rayDirs[bestI], rayStarts[bestJ], rayDirs[bestJ],
										 closestTime, worldPoint)
		|| closestTime < 0.f)
	{
		return false;
	}

	// Reject absurd distances (behind/way past the camera rig)
	if (glm::distance(rayStarts[bestI], worldPoint) > k_maxTriangulationDistMeters)
		return false;

	// Validate reprojection + cheirality across every observation
	float totalReprojError= 0.f;
	float maxReprojError= 0.f;
	for (size_t i= 0; i < obsCount; ++i)
	{
		glm::vec2 projected;
		if (!projectWorldPointToPixel(intrinsics, track.obsPoses[i], worldPoint, projected))
			return false; // behind a camera

		const float reproj= glm::distance(projected, track.obsPixels[i]);
		totalReprojError+= reproj;
		maxReprojError= std::max(maxReprojError, reproj);
	}

	if (maxReprojError > k_maxReprojErrorPx)
		return false;

	outPoint.worldPosition= worldPoint;
	outPoint.meanReprojErrorPx= totalReprojError / (float)obsCount;
	outPoint.observationCount= (int)obsCount;
	outPoint.maxParallaxDeg= bestParallaxDeg;

	return true;
}

bool cameraMovedEnoughForKeyframe(const glm::mat4& lastPose, const glm::mat4& currentPose)
{
	const glm::vec3 lastPos= lastPose[3];
	const glm::vec3 curPos= currentPose[3];
	if (glm::distance(lastPos, curPos) >= k_minKeyframeBaselineMeters)
		return true;

	const glm::vec3 lastForward= glm::normalize(glm::vec3(lastPose[2]));
	const glm::vec3 curForward= glm::normalize(glm::vec3(currentPose[2]));
	const float cosAngle= glm::clamp(glm::dot(lastForward, curForward), -1.f, 1.f);
	const float angleDeg= glm::degrees(acosf(cosAngle));

	return angleDeg >= k_minKeyframeAngleDeg;
}
} // namespace

// -- NaturalFeatureCloudBuilder -----
NaturalFeatureCloudBuilder::NaturalFeatureCloudBuilder(CameraComponentPtr cameraComponent,
													   VideoFrameDistortionView* distortionView)
	: m_state(new NaturalFeatureCloudState)
	, m_cameraComponent(cameraComponent)
	, m_distortionView(distortionView)
{
	m_state->init(cameraComponent, distortionView);
}

NaturalFeatureCloudBuilder::~NaturalFeatureCloudBuilder() { delete m_state; }

void NaturalFeatureCloudBuilder::resetCaptureState() { m_state->reset(); }

void NaturalFeatureCloudBuilder::setScreenRegionOfInterest(const glm::vec2& minPixel, const glm::vec2& maxPixel)
{
	m_state->hasRoi= true;
	m_state->roiMin= glm::min(minPixel, maxPixel);
	m_state->roiMax= glm::max(minPixel, maxPixel);
}

void NaturalFeatureCloudBuilder::clearScreenRegionOfInterest() { m_state->hasRoi= false; }

void NaturalFeatureCloudBuilder::processCurrentFrame()
{
	cv::Mat* gsBuffer= m_distortionView->getGrayscaleUndistortBuffer();
	if (gsBuffer == nullptr || gsBuffer->empty())
		return;

	// Own a copy: the distortion buffer is overwritten on the next readAndProcessVideoFrame()
	cv::Mat curGray= gsBuffer->clone();

	glm::mat4 cameraPose;
	const bool poseValid= m_cameraComponent->getStageSpaceAperturePose(cameraPose);

	// 1. Track existing features with Lucas-Kanade optical flow
	if (!m_state->prevGray.empty() && !m_state->tracks.empty())
	{
		std::vector<cv::Point2f> prevPts;
		prevPts.reserve(m_state->tracks.size());
		for (const FeatureTrack& track : m_state->tracks)
			prevPts.push_back(track.lastPixel);

		std::vector<cv::Point2f> nextPts;
		std::vector<uchar> status;
		std::vector<float> err;
		cv::calcOpticalFlowPyrLK(m_state->prevGray, curGray, prevPts, nextPts, status, err,
								 cv::Size(k_lkWindowSize, k_lkWindowSize), k_lkPyramidLevels);

		std::vector<FeatureTrack> surviving;
		surviving.reserve(m_state->tracks.size());
		for (size_t i= 0; i < m_state->tracks.size(); ++i)
		{
			if (status[i] != 0 && err[i] < k_maxLkErrorPx
				&& isPixelInFrame(nextPts[i], m_state->frameWidth, m_state->frameHeight)
				&& isPixelInRoi(m_state, nextPts[i]))
			{
				m_state->tracks[i].lastPixel= nextPts[i];
				surviving.push_back(std::move(m_state->tracks[i]));
			}
		}
		m_state->tracks.swap(surviving);
	}

	// 2. Keyframe gating: append an observation and attempt triangulation on qualifying frames
	if (poseValid)
	{
		const bool isKeyframe=
			!m_state->hasKeyframe || cameraMovedEnoughForKeyframe(m_state->lastKeyframePose, cameraPose);
		if (isKeyframe)
		{
			m_state->hasKeyframe= true;
			m_state->lastKeyframePose= cameraPose;
			m_state->keyframeCount++;

			for (FeatureTrack& track : m_state->tracks)
			{
				track.obsPoses.push_back(cameraPose);
				track.obsPixels.push_back(glm::vec2(track.lastPixel.x, track.lastPixel.y));

				// Bound observation history (keep the most recent keyframes)
				if ((int)track.obsPoses.size() > k_maxObservationsPerTrack)
				{
					track.obsPoses.erase(track.obsPoses.begin());
					track.obsPixels.erase(track.obsPixels.begin());
				}

				if (!track.triangulated)
				{
					NaturalFeaturePoint point;
					if (triangulateTrack(m_state->inputCameraIntrinsics, track, point))
					{
						track.triangulated= true;
						m_state->cloud.push_back(point);
					}
				}
			}
		}
	}

	// 3. Replenish features up to the target count within the ROI, spaced away from existing tracks
	if ((int)m_state->tracks.size() < k_targetFeatureCount)
	{
		cv::Mat mask(curGray.size(), CV_8UC1, cv::Scalar(255));
		if (m_state->hasRoi)
		{
			mask.setTo(cv::Scalar(0));
			const int x= std::max(0, (int)m_state->roiMin.x);
			const int y= std::max(0, (int)m_state->roiMin.y);
			const int w= std::min(curGray.cols - x, (int)(m_state->roiMax.x - m_state->roiMin.x));
			const int h= std::min(curGray.rows - y, (int)(m_state->roiMax.y - m_state->roiMin.y));
			if (w > 0 && h > 0)
				mask(cv::Rect(x, y, w, h)).setTo(cv::Scalar(255));
		}
		// Exclude neighborhoods of existing tracks so new corners spread out
		for (const FeatureTrack& track : m_state->tracks)
		{
			cv::circle(mask, track.lastPixel, (int)k_goodFeaturesMinDistancePx, cv::Scalar(0), -1);
		}

		std::vector<cv::Point2f> corners;
		cv::goodFeaturesToTrack(curGray, corners, k_targetFeatureCount - (int)m_state->tracks.size(),
								k_goodFeaturesQualityLevel, k_goodFeaturesMinDistancePx, mask);
		for (const cv::Point2f& corner : corners)
		{
			FeatureTrack track;
			track.lastPixel= corner;
			m_state->tracks.push_back(std::move(track));
		}
	}

	// 4. Save current frame for next iteration
	m_state->prevGray= curGray;
}

bool NaturalFeatureCloudBuilder::hasUsableCloud() const
{
	const CloudBuildStats stats= getStats();
	return stats.cloudPointCount >= k_minUsableCloudPoints && stats.coverageMeters >= k_minUsableCoverageMeters;
}

const std::vector<NaturalFeaturePoint>& NaturalFeatureCloudBuilder::getCloudPoints() const { return m_state->cloud; }

CloudBuildStats NaturalFeatureCloudBuilder::getStats() const
{
	CloudBuildStats stats;
	stats.trackedFeatureCount= (int)m_state->tracks.size();
	stats.cloudPointCount= (int)m_state->cloud.size();
	stats.keyframeCount= m_state->keyframeCount;

	if (!m_state->cloud.empty())
	{
		glm::vec3 minBounds= m_state->cloud[0].worldPosition;
		glm::vec3 maxBounds= m_state->cloud[0].worldPosition;
		float totalReproj= 0.f;
		for (const NaturalFeaturePoint& point : m_state->cloud)
		{
			minBounds= glm::min(minBounds, point.worldPosition);
			maxBounds= glm::max(maxBounds, point.worldPosition);
			totalReproj+= point.meanReprojErrorPx;
		}
		stats.meanReprojErrorPx= totalReproj / (float)m_state->cloud.size();
		stats.coverageMeters= glm::distance(minBounds, maxBounds);
	}

	return stats;
}

void NaturalFeatureCloudBuilder::renderTrackedFeatures2d()
{
	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();
	const float frameWidth= (float)m_state->frameWidth;
	const float frameHeight= (float)m_state->frameHeight;

	std::vector<glm::vec3> triangulatedPts;
	std::vector<glm::vec3> pendingPts;
	for (const FeatureTrack& track : m_state->tracks)
	{
		const glm::vec3 p(track.lastPixel.x, track.lastPixel.y, 0.5f);
		if (track.triangulated)
			triangulatedPts.push_back(p);
		else
			pendingPts.push_back(p);
	}

	if (!pendingPts.empty())
		drawPointList2d(graphicsContext, frameWidth, frameHeight, pendingPts.data(), (int)pendingPts.size(),
						Colors::Yellow, 2.f);
	if (!triangulatedPts.empty())
		drawPointList2d(graphicsContext, frameWidth, frameHeight, triangulatedPts.data(), (int)triangulatedPts.size(),
						Colors::Green, 2.f);
}

void NaturalFeatureCloudBuilder::renderCloudPoints3d()
{
	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();

	for (const NaturalFeaturePoint& point : m_state->cloud)
	{
		drawPoint(graphicsContext, glm::mat4(1.f), point.worldPosition, Colors::Cyan, 3.f);
	}
}
