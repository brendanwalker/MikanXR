#pragma once

#include <memory>

class VideoCapabilitiesConfig;
using VideoCapabilitiesConfigPtr = std::shared_ptr<VideoCapabilitiesConfig>;
using VideoCapabilitiesConfigConstPtr = std::shared_ptr<const VideoCapabilitiesConfig>;

class OpenCVVideoConfig;
using OpenCVVideoConfigPtr = std::shared_ptr<OpenCVVideoConfig>;
using OpenCVVideoConfigConstPtr = std::shared_ptr<const OpenCVVideoConfig>;

class WMFVideoConfig;
using WMFVideoConfigPtr = std::shared_ptr<WMFVideoConfig>;
using WMFVideoConfigConstPtr = std::shared_ptr<const WMFVideoConfig>;

class WMFMonoVideoConfig;
using WMFMonoVideoConfigPtr = std::shared_ptr<WMFMonoVideoConfig>;
using WMFMonoVideoConfigConstPtr = std::shared_ptr<const WMFMonoVideoConfig>;

class WMFStereoVideoConfig;
using WMFStereoVideoConfigPtr = std::shared_ptr<WMFStereoVideoConfig>;
using WMFStereoVideoConfigConstPtr = std::shared_ptr<const WMFStereoVideoConfig>;