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

bool GStreamerVideoConfig::applyDevicePath(const std::string& devicePath)
{
	// Parse the device path
	std::string protocolStr;
	GStreamerProtocol protocolEnum;
	std::string addressStr;
	std::string pathStr;
	int portValue= 0;

	// Find the protocol
	size_t protocolEnd = devicePath.find("://");
	if (protocolEnd != std::string::npos)
	{
		protocolStr = devicePath.substr(0, protocolEnd);
	}

	// Find the address
	size_t addressStart = protocolEnd + 3;
	size_t addressEnd = devicePath.find(":", addressStart);
	if (addressEnd != std::string::npos)
	{
		addressStr = devicePath.substr(addressStart, addressEnd - addressStart);
	}

	// Find the port
	size_t portStart = addressEnd + 1;
	size_t portEnd = devicePath.find("/", portStart);
	if (portEnd != std::string::npos)
	{
		portValue = std::stoi(devicePath.substr(portStart, portEnd - portStart));
	}

	// Find the path
	size_t pathStart = portEnd + 1;
	pathStr = devicePath.substr(pathStart);

	// Apply the parsed values
	if (protocolStr == "rtmp")
	{
		protocolEnum = GStreamerProtocol::RTMP;
	}
	else if (protocolStr == "rtsp")
	{
		protocolEnum = GStreamerProtocol::RTSP;
	}
	else
	{
		protocolEnum = GStreamerProtocol::INVALID;
	}

	// Apply the parsed values if the address and port are valid
	if (protocolEnum != protocol || 
		addressStr != address ||
		portValue != port ||
		pathStr != path)
	{
		protocol = protocolEnum;
		address = addressStr;
		path = pathStr;
		port = portValue;

		return true;
	}

	return false;
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