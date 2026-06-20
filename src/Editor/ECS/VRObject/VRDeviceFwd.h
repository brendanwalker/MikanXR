#pragma once

#include <memory>

class VRDeviceDefinition;
using VRDeviceDefinitionPtr= std::shared_ptr<VRDeviceDefinition>;
using VRDeviceDefinitionConstPtr= std::shared_ptr<const VRDeviceDefinition>;
using VRDeviceDefinitionWeakPtr= std::weak_ptr<VRDeviceDefinition>;

class VRDeviceComponent;
using VRDeviceComponentPtr= std::shared_ptr<VRDeviceComponent>;
using VRDeviceComponentConstPtr= std::shared_ptr<const VRDeviceComponent>;
using VRDeviceComponentConstWeakPtr= std::weak_ptr<const VRDeviceComponent>;
using VRDeviceComponentWeakPtr= std::weak_ptr<VRDeviceComponent>;

class VRDevicePoseView;
using VRDevicePoseViewPtr= std::shared_ptr<VRDevicePoseView>;