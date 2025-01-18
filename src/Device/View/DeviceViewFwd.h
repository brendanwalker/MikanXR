#pragma once

#include <memory>

class DeviceView;
using DeviceViewPtr = std::shared_ptr<DeviceView>;

class VideoSourceView;
using VideoSourceViewPtr = std::shared_ptr<VideoSourceView>;

class VRDeviceView;
using VRDeviceViewPtr = std::shared_ptr<VRDeviceView>;
using VRDeviceViewConstWeakPtr = std::weak_ptr<const VRDeviceView>;

class VRDevicePoseView;
using VRDevicePoseViewPtr = std::shared_ptr<VRDevicePoseView>;
