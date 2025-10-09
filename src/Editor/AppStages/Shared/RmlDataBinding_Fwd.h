#pragma once

#include <memory>
#include <string>

template <typename t_element_type> class RmlDataBinding_List;

using RmlDataBinding_ComponentIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ComponentIdListPtr = std::shared_ptr<RmlDataBinding_ComponentIdList>;

using RmlDataBinding_ArucoIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ArucoIdListPtr = std::shared_ptr<RmlDataBinding_ArucoIdList>;

using RmlDataBinding_VRDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VRDevicePathListPtr = std::shared_ptr<RmlDataBinding_VRDevicePathList>;

using RmlDataBinding_SocketNameList = RmlDataBinding_List<std::string>;
using RmlDataBinding_SocketNameListPtr = std::shared_ptr<RmlDataBinding_SocketNameList>;

using RmlDataBinding_USBDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_USBDevicePathListPtr = std::shared_ptr<RmlDataBinding_USBDevicePathList>;

using RmlDataBinding_VideoModeList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VideoModeListPtr = std::shared_ptr<RmlDataBinding_VideoModeList>;

class RmlModel_AnchorComponent;
using RmlModel_AnchorComponentPtr = std::shared_ptr<RmlModel_AnchorComponent>;

class RmlModel_CameraComponent;
using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;

class RmlModel_MarkerComponent;
using RmlModel_MarkerComponentPtr = std::shared_ptr<RmlModel_MarkerComponent>;

class RmlModel_MarkerObjectSystem;
using RmlModel_MarkerObjectSystemPtr = std::shared_ptr<RmlModel_MarkerObjectSystem>;

class RmlModel_MikanComponent;
using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;

class RmlModel_PropertyInterface;
using RmlModel_PropertyInterfacePtr = std::shared_ptr<RmlModel_PropertyInterface>;

class RmlModel_StageComponent;
using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;

class RmlModel_StencilComponent;
using RmlModel_StencilComponentPtr = std::shared_ptr<RmlModel_StencilComponent>;

class RmlModel_MarkerTrackingVolumeComponent;
using RmlModel_MarkerTrackingVolumeComponentPtr = std::shared_ptr<RmlModel_MarkerTrackingVolumeComponent>;

class RmlModel_VRTrackingVolumeComponent;
using RmlModel_VRTrackingVolumeComponentPtr = std::shared_ptr<RmlModel_VRTrackingVolumeComponent>;

class RmlModel_TrackingMountComponent;
using RmlModel_TrackingMountComponentPtr = std::shared_ptr<RmlModel_TrackingMountComponent>;

class RmlModel_USBVideoSourceComponent;
using RmlModel_USBVideoSourceComponentPtr = std::shared_ptr<RmlModel_USBVideoSourceComponent>;