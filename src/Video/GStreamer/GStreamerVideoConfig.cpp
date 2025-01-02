#include "GStreamerVideoConfig.h"
#include "VideoCapabilitiesConfig.h"

#include "glm/gtc/type_ptr.hpp"

#include <sstream>

GStreamerVideoConfig::GStreamerVideoConfig(const std::string& fnamebase)
	: CommonVideoConfig(fnamebase)
{
	CommonVideoConfig::current_mode = "640x480(60FPS)";

	memset(video_properties, 0, sizeof(video_properties));

	cameraIntrinsics.camera_matrix = {
	   604.1783809256024, 0.0, 320.0,
	   0.0, 604.1783809256024, 240.0,
	   0.0, 0.0, 1.0
	};

	cameraIntrinsics.hfov = 60.0; // degrees
	cameraIntrinsics.vfov = 45.0; // degrees
	cameraIntrinsics.znear = 10.0; // cm
	cameraIntrinsics.zfar = 200.0; // cm
	cameraIntrinsics.distortion_coefficients.k1 = 0.015861621530855057;
	cameraIntrinsics.distortion_coefficients.k2 = 0.09840206266336657;
	cameraIntrinsics.distortion_coefficients.k3 = 0.0;
	cameraIntrinsics.distortion_coefficients.k4 = 0.0;
	cameraIntrinsics.distortion_coefficients.k5 = 0.0;
	cameraIntrinsics.distortion_coefficients.k6 = 0.4512905272272733;
	cameraIntrinsics.distortion_coefficients.p1 = 0.0;
	cameraIntrinsics.distortion_coefficients.p2 = 0.0;
};

configuru::Config GStreamerVideoConfig::writeToJSON()
{
	configuru::Config pt = CommonVideoConfig::writeToJSON();

	writeMatrix3d(pt, "camera_matrix", cameraIntrinsics.camera_matrix);
	writeDistortionCoefficients(pt, "distortion", &cameraIntrinsics.distortion_coefficients);

	return pt;
}

void GStreamerVideoConfig::readFromJSON(const configuru::Config& pt)
{
	CommonVideoConfig::readFromJSON(pt);

	cameraIntrinsics.hfov = pt.get_or<float>("hfov", 60.0f);
	cameraIntrinsics.vfov = pt.get_or<float>("vfov", 45.0f);
	cameraIntrinsics.znear = pt.get_or<float>("zNear", 10.0f);
	cameraIntrinsics.zfar = pt.get_or<float>("zFar", 200.0f);
	cameraIntrinsics.pixel_width = pt.get_or<float>("frame_width", 640.f);
	cameraIntrinsics.pixel_height = pt.get_or<float>("frame_height", 480.f);

	readMatrix3d(pt, "camera_matrix", cameraIntrinsics.camera_matrix);
	readDistortionCoefficients(pt, "distortion",
							   &cameraIntrinsics.distortion_coefficients,
							   &cameraIntrinsics.distortion_coefficients);
}

bool GStreamerVideoConfig::buildGStreamerPipelineString(std::string& outPipelineString) const
{
	// TODO: Build a GStreamer pipeline string based on the configuration
	std::stringstream ss;
	ss << "rtspsrc location = \"rtsp://192.168.1.134:8554/cam\" latency = 0 buffer - mode = auto !rtph264depay !h264parse !d3d11h264dec !video.";
	outPipelineString = ss.str();

	return true;
}
