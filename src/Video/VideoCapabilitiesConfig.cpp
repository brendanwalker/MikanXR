// -- includes -----
#include "VideoCapabilitiesConfig.h"
#include "PathUtils.h"

const configuru::Config VideoFrameSectionInfo::writeToJSON() const
{
	configuru::Config pt{
        {"x", x},
		{"y", y},
    };

	return pt;
}

void VideoFrameSectionInfo::readFromJSON(const configuru::Config &pt)
{
    x = pt.get_or<int>("x", x);
    y = pt.get_or<int>("y", y);
}

const configuru::Config VideoModeConfig::writeToJSON() const
{
    configuru::Config pt{
        {"mode_name", modeName},
		{"frame_rate", frameRate},
		{"is_frame_mirrored", isFrameMirrored},
		{"is_buffer_mirrored", isBufferMirrored},
		{"buffer_pixel_width", bufferPixelWidth},
		{"buffer_pixel_height", bufferPixelHeight},
		{"buffer_format", bufferFormat}
    };

	switch (intrinsics.intrinsics_type)
	{
	case MONO_CAMERA_INTRINSICS:
		pt["intrinsics_type"]= std::string("mono");
		CommonConfig::writeMonoTrackerIntrinsics(pt, intrinsics.getMonoIntrinsics());
		break;
	case STEREO_CAMERA_INTRINSICS:
		pt["intrinsics_type"]= std::string("stereo");
		CommonConfig::writeStereoTrackerIntrinsics(pt, intrinsics.getStereoIntrinsics());
		break;
	}

	std::vector<configuru::Config> frame_section_configs;
	for (const VideoFrameSectionInfo &section_bounds : frameSections)
	{
		frame_section_configs.push_back(section_bounds.writeToJSON());
	}
	pt.insert_or_assign(std::string("frame_sections"), frame_section_configs);

	return pt;
}

void VideoModeConfig::readFromJSON(const configuru::Config &pt)
{
    modeName = pt.get_or<std::string>("mode_name", modeName);
    frameRate = pt.get_or<float>("frame_rate", frameRate);
	isFrameMirrored = pt.get_or<bool>("is_frame_mirrored", isFrameMirrored);
	isBufferMirrored = pt.get_or<bool>("is_buffer_mirrored", isBufferMirrored);
	bufferFormat = pt.get_or<std::string>("buffer_format", CAMERA_BUFFER_FORMAT_MJPG);

	std::string intrinsics_type= pt.get_or<std::string>("intrinsics_type", "");
	if (intrinsics_type == "mono")
	{
		auto monoIntrinsics= intrinsics.getMonoIntrinsics();
		CommonConfig::readMonoTrackerIntrinsics(pt, monoIntrinsics);

		bufferPixelWidth= pt.get_or<int>("buffer_pixel_width", (int)monoIntrinsics.pixel_width);
		bufferPixelHeight= pt.get_or<int>("buffer_pixel_height", (int)monoIntrinsics.pixel_height);

		intrinsics.setMonoIntrinsics(monoIntrinsics);
	}
	else if (intrinsics_type == "stereo")
	{
		auto stereoIntrinsics= intrinsics.getStereoIntrinsics();
		CommonConfig::readStereoTrackerIntrinsics(pt, stereoIntrinsics);
		intrinsics.intrinsics_type= STEREO_CAMERA_INTRINSICS;

		bufferPixelWidth= pt.get_or<int>("buffer_pixel_width", (int)stereoIntrinsics.pixel_width);
		bufferPixelHeight= pt.get_or<int>("buffer_pixel_height", (int)stereoIntrinsics.pixel_height);

		intrinsics.setStereoIntrinsics(stereoIntrinsics);
	}

	if (pt.has_key("frame_sections"))
	{
		for (const configuru::Config& element : pt["frame_sections"].as_array())
		{
			VideoFrameSectionInfo section_bounds;
			section_bounds.readFromJSON(element);
			frameSections.push_back(section_bounds);
		}
	}
}

VideoCapabilitiesConfig::VideoCapabilitiesConfig(const std::string &fnamebase)
	: CommonConfig(fnamebase)
	, friendlyName()
	, usbProductId(0x0000)
	, usbVendorId(0x0000)
	, deviceType(eDeviceType::MonoVideoSource)
{
}

configuru::Config VideoCapabilitiesConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["friendly_name"]= friendlyName;
	pt["usb_product_id"]= usbProductId;
	pt["usb_vendor_id"]= usbVendorId;

	writeDeviceType(pt, "device_type", deviceType);

	std::vector<configuru::Config> modes;
	for (const VideoModeConfig &mode : supportedModes)
	{
		modes.push_back(mode.writeToJSON());
	}
	pt.insert_or_assign(std::string("supported_modes"), modes);

	return pt;
}

void VideoCapabilitiesConfig::readFromJSON(const configuru::Config &pt)
{
	CommonConfig::readFromJSON(pt);

	friendlyName= pt.get_or<std::string>("friendly_name", friendlyName);
	usbProductId= pt.get_or<int>("usb_product_id", usbProductId);
	usbVendorId= pt.get_or<int>("usb_vendor_id", usbVendorId);

	readDeviceType(pt, "device_type", deviceType);

	for (const configuru::Config& element : pt["supported_modes"].as_array())
	{
		VideoModeConfig mode;
		mode.readFromJSON(element);
		supportedModes.push_back(mode);
	}
}

int VideoCapabilitiesConfig::findVideoModeIndex(const std::string& mode_name) const
{
	for (int videoModeIndex= 0; videoModeIndex < supportedModes.size(); ++videoModeIndex)
	{
		const VideoModeConfig& mode_config= supportedModes[videoModeIndex];

		if (mode_config.modeName == mode_name)
		{
			return videoModeIndex;
		}
	}

	return -1;
}

const VideoModeConfig *VideoCapabilitiesConfig::findVideoMode(const std::string &mode_name) const
{
	int videoModeIndex= findVideoModeIndex(mode_name);
	if (videoModeIndex != -1)
	{
		return &supportedModes[videoModeIndex];
	}

	return nullptr;
}

const VideoModeConfig* VideoCapabilitiesConfig::findMostCompatibleVideoMode(
	int width, 
	int height, 
	float fps,
	const std::string& bufferFormat) const
{
	const VideoModeConfig* best_mode= nullptr;
	int best_score= 0;

	for (const VideoModeConfig& mode_config : supportedModes)
	{
		int score= 0;

		if (mode_config.bufferPixelWidth == width && mode_config.bufferPixelHeight == height)
		{
			score+= 1;
		}

		if (mode_config.frameRate == fps)
		{
			score+= 1;
		}

		if (mode_config.bufferFormat == bufferFormat)
		{
			score += 1;
		}

		if (score > best_score)
		{
			best_score= score;
			best_mode= &mode_config;
		}
	}

	return best_mode;

}

void VideoCapabilitiesConfig::getAvailableVideoModes(std::vector<std::string> &out_mode_names) const
{
	out_mode_names.clear();
	for (const VideoModeConfig &mode_config : supportedModes)
	{
		out_mode_names.push_back(mode_config.modeName);
	}
}

bool VideoCapabilitiesSet::reloadSupportedVideoCapabilities()
{
	const std::filesystem::path capability_directory= PathUtils::getResourceDirectory() / std::string("supported_trackers");
	const std::vector<std::string> filenames= PathUtils::listFilenamesInDirectory(capability_directory, ".json");

	m_supportedTrackers.clear();
	for (std::string filename : filenames)
	{
		const std::filesystem::path filepath= capability_directory / filename;
		VideoCapabilitiesConfigPtr config = std::make_shared<VideoCapabilitiesConfig>(filename);

		if (config->load(filepath))
		{
			m_supportedTrackers.push_back(config);
		}
	}

    return m_supportedTrackers.size() > 0;
}

VideoCapabilitiesConfigConstPtr VideoCapabilitiesSet::getVideoSourceCapabilities(
	unsigned short vendor_id, unsigned short product_id) const
{
	for (auto filter : m_supportedTrackers)
	{
		if (filter->usbVendorId == vendor_id && filter->usbProductId == product_id)
		{
			return filter;
		}
	}

	return nullptr;
}