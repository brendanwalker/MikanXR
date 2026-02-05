#include "SharedTextureUtility.h"

#include <sstream>

bool makeSpoutSenderName(
	const std::string prefix,
	MikanCameraID camera_id,
	SharedTextureType buffer_type,
	std::string& outSenderName)
{
	std::string buffer_name;
	switch (buffer_type)
	{
	case SharedTextureType::COLOR:
		buffer_name = "color";
		break;
	case SharedTextureType::DEPTH:
		buffer_name = "depth";
	default:
		return false;
	}

	std::stringstream ss;
	ss << prefix;
	ss << "_";
	if (camera_id >= 0)
	{
		ss << "camera";
		ss << camera_id;
		ss << "_";
	}
	ss << buffer_name;

	outSenderName= ss.str();

	return true;
}