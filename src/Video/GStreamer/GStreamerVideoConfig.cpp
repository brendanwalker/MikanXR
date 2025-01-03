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

std::string GStreamerVideoConfig::getSourcePluginString() const
{
	std::string result;

	switch (protocol)
	{
		case GStreamerProtocol::RTMP:
			result = "rtmpsrc";
			break;
		case GStreamerProtocol::RTSP:
			result = "rtspsrc";
			break;
		default:
			result = "UNKNOWN";
	}

	return result;
}

std::string GStreamerVideoConfig::getURIProtocolString() const
{
	std::string result;

	switch (protocol)
	{
		case GStreamerProtocol::RTMP:
			result = "rtmp";
			break;
		case GStreamerProtocol::RTSP:
			result = "rtsp";
			break;
		default:
			result = "UNKNOWN";
	}

	return result;
}

std::string GStreamerVideoConfig::getFullURIPath() const
{
	std::stringstream ss;

	ss << getURIProtocolString() << "://" << address << ":" << port << "/" << path;

	return ss.str();
}

std::string GStreamerVideoConfig::getCodecString() const
{
	std::string result;

	switch (codec)
	{
		case GStreamerCodec::MJPEG:
			result = "MJPEG";
			break;
		case GStreamerCodec::H264:
			result = "H264";
			break;
		case GStreamerCodec::H265:
			result = "H265";
			break;
		default:
			result = "UNKNOWN";
	}

	return result;
}