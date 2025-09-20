#pragma once

#include <memory>
#include <string>

template <typename t_element_type> class RmlDataBinding_List;

using RmlDataBinding_ComponentIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ComponentIdListPtr = std::shared_ptr<RmlDataBinding_ComponentIdList>;

using RmlDataBinding_VRDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VRDevicePathListPtr = std::shared_ptr<RmlDataBinding_VRDevicePathList>;

class RmlModel_AnchorComponent;
using RmlModel_AnchorComponentPtr = std::shared_ptr<RmlModel_AnchorComponent>;

class RmlModel_CameraComponent;
using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;

class RmlModel_MikanComponent;
using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;

class RmlModel_PropertyInterface;
using RmlModel_PropertyInterfacePtr = std::shared_ptr<RmlModel_PropertyInterface>;

class RmlModel_StageComponent;
using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;

class RmlModel_StencilComponent;
using RmlModel_StencilComponentPtr = std::shared_ptr<RmlModel_StencilComponent>;

class RmlModel_TrackingVolumeComponent;
using RmlModel_TrackingVolumeComponentPtr = std::shared_ptr<RmlModel_TrackingVolumeComponent>;