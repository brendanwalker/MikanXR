// -- includes -----
#include "CommonVideoConfig.h"

// -- WMF Stereo Tracker Config
CommonVideoConfig::CommonVideoConfig(const std::string &fnamebase)
    : CommonConfig(fnamebase)
    , is_valid(false)
    , max_poll_failure_count(100)
	, current_mode("")
{
    orientationOffset= {1.0, 0.0, 0.0, 0.0};
	positionOffset= {0.0, 0.0, 0.0};

	memset(video_properties, 0, sizeof(video_properties));
};

configuru::Config CommonVideoConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	pt["is_valid"]= is_valid;
	pt["max_poll_failure_count"]= max_poll_failure_count;
	pt["current_mode"]= current_mode;
	pt["brightness"]= video_properties[(int)VideoPropertyType::Brightness];
	pt["contrast"]= video_properties[(int)VideoPropertyType::Contrast];
	pt["hue"]= video_properties[(int)VideoPropertyType::Hue];
	pt["saturation"]= video_properties[(int)VideoPropertyType::Saturation];
	pt["sharpness"]= video_properties[(int)VideoPropertyType::Sharpness];
	pt["gamma"]= video_properties[(int)VideoPropertyType::Gamma];
	pt["whitebalance"]= video_properties[(int)VideoPropertyType::WhiteBalance];
	pt["redbalance"]= video_properties[(int)VideoPropertyType::RedBalance];
	pt["greenbalance"]= video_properties[(int)VideoPropertyType::GreenBalance];
	pt["bluebalance"]= video_properties[(int)VideoPropertyType::BlueBalance];
	pt["gain"]= video_properties[(int)VideoPropertyType::Gain];
	pt["pan"]= video_properties[(int)VideoPropertyType::Pan];
	pt["tilt"]= video_properties[(int)VideoPropertyType::Tilt];
	pt["roll"]= video_properties[(int)VideoPropertyType::Roll];
	pt["zoom"]= video_properties[(int)VideoPropertyType::Zoom];
	pt["exposure"]= video_properties[(int)VideoPropertyType::Exposure];
	pt["iris"]= video_properties[(int)VideoPropertyType::Iris];
	pt["focus"]= video_properties[(int)VideoPropertyType::Focus];

	writeQuaderntiond(pt, "orientationOffset", orientationOffset);
	writeVector3d(pt, "positionOffset", positionOffset);

    return pt;
}

void CommonVideoConfig::readFromJSON(const configuru::Config &pt)
{
	CommonConfig::readFromJSON(pt);

    is_valid = pt.get_or<bool>("is_valid", false);
    max_poll_failure_count = pt.get_or<long>("max_poll_failure_count", 100);
	current_mode= pt.get_or<std::string>("current_mode", current_mode);

    video_properties[(int)VideoPropertyType::Brightness]= (int)pt.get_or<float>("brightness", (float)video_properties[(int)VideoPropertyType::Brightness]);
	video_properties[(int)VideoPropertyType::Contrast]= (int)pt.get_or<float>("contrast", (float)video_properties[(int)VideoPropertyType::Contrast]);
	video_properties[(int)VideoPropertyType::Hue]= (int)pt.get_or<float>("hue", (float)video_properties[(int)VideoPropertyType::Hue]);
	video_properties[(int)VideoPropertyType::Saturation]= (int)pt.get_or<float>("saturation", (float)video_properties[(int)VideoPropertyType::Saturation]);
	video_properties[(int)VideoPropertyType::Sharpness]= (int)pt.get_or<float>("sharpness", (float)video_properties[(int)VideoPropertyType::Sharpness]);
	video_properties[(int)VideoPropertyType::Gamma]= (int)pt.get_or<float>("gamma", (float)video_properties[(int)VideoPropertyType::Gamma]);
	video_properties[(int)VideoPropertyType::WhiteBalance]= (int)pt.get_or<float>("whitebalance", (float)video_properties[(int)VideoPropertyType::WhiteBalance]);
	video_properties[(int)VideoPropertyType::RedBalance]= (int)pt.get_or<float>("redbalance", (float)video_properties[(int)VideoPropertyType::RedBalance]);
	video_properties[(int)VideoPropertyType::GreenBalance]= (int)pt.get_or<float>("greenbalance", (float)video_properties[(int)VideoPropertyType::GreenBalance]);
	video_properties[(int)VideoPropertyType::BlueBalance]= (int)pt.get_or<float>("bluebalance", (float)video_properties[(int)VideoPropertyType::BlueBalance]);
	video_properties[(int)VideoPropertyType::Gain]= (int)pt.get_or<float>("gain", (float)video_properties[(int)VideoPropertyType::Gain]);
	video_properties[(int)VideoPropertyType::Pan]= (int)pt.get_or<float>("pan", (float)video_properties[(int)VideoPropertyType::Pan]);
	video_properties[(int)VideoPropertyType::Tilt]= (int)pt.get_or<float>("tilt", (float)video_properties[(int)VideoPropertyType::Tilt]);
	video_properties[(int)VideoPropertyType::Roll]= (int)pt.get_or<float>("roll", (float)video_properties[(int)VideoPropertyType::Roll]);
	video_properties[(int)VideoPropertyType::Zoom]= (int)pt.get_or<float>("zoom", (float)video_properties[(int)VideoPropertyType::Zoom]);
	video_properties[(int)VideoPropertyType::Exposure]= (int)pt.get_or<float>("exposure", (float)video_properties[(int)VideoPropertyType::Exposure]);
	video_properties[(int)VideoPropertyType::Iris]= (int)pt.get_or<float>("iris", (float)video_properties[(int)VideoPropertyType::Iris]);
	video_properties[(int)VideoPropertyType::Focus]= (int)pt.get_or<float>("focus", (float)video_properties[(int)VideoPropertyType::Focus]);

	readQuaterniond(pt, "orientationOffset", orientationOffset);
	readVector3d(pt, "positionOffset", positionOffset);
}