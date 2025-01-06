#include "GStreamerVideoConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "CameraMath.h"
#include "StringUtils.h"

#include "glm/gtc/type_ptr.hpp"

#include <sstream>

const std::string g_GStreamerProtocolStrings[(int)eGStreamerProtocol::COUNT] = {
	"rtmp",
	"rtsp"
};
const std::string* k_GStreamerProtocolStrings = g_GStreamerProtocolStrings;

GStreamerVideoConfig::GStreamerVideoConfig(const std::string& fnamebase)
	: CommonVideoConfig(fnamebase)
{
	protocol = eGStreamerProtocol::INVALID;
	address = "";
	path = "";
	port = 0;
	createDefautMonoIntrinsics(1080, 720, cameraIntrinsics);
};

configuru::Config GStreamerVideoConfig::writeToJSON()
{
	configuru::Config pt = CommonVideoConfig::writeToJSON();

	pt["protocol"] = 
		protocol != eGStreamerProtocol::INVALID 
		? g_GStreamerProtocolStrings[(int)protocol]
		: "INVALID";
	pt["address"] = address;
	pt["path"] = path;
	pt["port"] = port;

	writeMonoTrackerIntrinsics(pt, cameraIntrinsics);

	return pt;
}

void GStreamerVideoConfig::readFromJSON(const configuru::Config& pt)
{
	CommonVideoConfig::readFromJSON(pt);

	protocol =
		StringUtils::FindEnumValue<eGStreamerProtocol>(
			pt.get_or<std::string>("protocol", "INVALID"),
			g_GStreamerProtocolStrings);
	address= pt.get_or<std::string>("address", "");
	path= pt.get_or<std::string>("path", "");
	port= pt.get_or<int>("port", 0);

	readMonoTrackerIntrinsics(pt, cameraIntrinsics);
}

bool GStreamerVideoConfig::applyDevicePath(const std::string& devicePath)
{
	CommonVideoConfig::current_mode = devicePath;

	// Parse the device path
	std::string protocolStr;
	eGStreamerProtocol protocolEnum;
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
	protocolEnum =
		StringUtils::FindEnumValue<eGStreamerProtocol>(
			protocolStr,
			g_GStreamerProtocolStrings);

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
	if (protocol != eGStreamerProtocol::INVALID)
	{
		return g_GStreamerProtocolStrings[(int)protocol] + "src";
	}

	return "UNKNOWN";
}

std::string GStreamerVideoConfig::getURIProtocolString() const
{
	if (protocol != eGStreamerProtocol::INVALID)
	{
		return g_GStreamerProtocolStrings[(int)protocol];
	}

	return "UNKNOWN";
}

std::string GStreamerVideoConfig::getFullURIPath() const
{
	std::stringstream ss;

	ss << getURIProtocolString() << "://" << address << ":" << port << "/" << path;

	return ss.str();
}