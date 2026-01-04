#pragma once

#include <memory>
#include <string>

template <typename t_element_type> class RmlDataBinding_List;

using RmlDataBinding_ComponentIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ComponentIdListPtr = std::shared_ptr<RmlDataBinding_ComponentIdList>;

using RmlDataBinding_ScriptTriggerList = RmlDataBinding_List<std::string>;
using RmlDataBinding_ScriptTriggerListPtr = std::shared_ptr<RmlDataBinding_ScriptTriggerList>;

using RmlDataBinding_ArucoIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ArucoIdListPtr = std::shared_ptr<RmlDataBinding_ArucoIdList>;

using RmlDataBinding_VRDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VRDevicePathListPtr = std::shared_ptr<RmlDataBinding_VRDevicePathList>;

using RmlDataBinding_SocketNameList = RmlDataBinding_List<std::string>;
using RmlDataBinding_SocketNameListPtr = std::shared_ptr<RmlDataBinding_SocketNameList>;

using RmlDataBinding_SpoutSourceList = RmlDataBinding_List<std::string>;
using RmlDataBinding_SpoutSourceListPtr = std::shared_ptr<RmlDataBinding_SpoutSourceList>;

using RmlDataBinding_USBDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_USBDevicePathListPtr = std::shared_ptr<RmlDataBinding_USBDevicePathList>;

using RmlDataBinding_VideoModeList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VideoModeListPtr = std::shared_ptr<RmlDataBinding_VideoModeList>;

class RmlDataBinding_VideoSourceSetting;
using RmlDataBinding_VideoSourceSettingPtr = std::shared_ptr<RmlDataBinding_VideoSourceSetting>;

class RmlModel_VideoSourceSettings;
using RmlModel_VideoSourceSettingsPtr = std::shared_ptr<RmlModel_VideoSourceSettings>;

class RmlModel_AnchorComponent;
using RmlModel_AnchorComponentPtr = std::shared_ptr<RmlModel_AnchorComponent>;

class RmlModel_BoxStencilComponent;
using RmlModel_BoxStencilComponentPtr = std::shared_ptr<RmlModel_BoxStencilComponent>;

class RmlModel_CameraComponent;
using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;

class RmlModel_ClientTextureSourceComponent;
using RmlModel_ClientTextureSourceComponentPtr = std::shared_ptr<RmlModel_ClientTextureSourceComponent>;

class RmlModel_CompositorComponent;
using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;

class RmlModel_MarkerComponent;
using RmlModel_MarkerComponentPtr = std::shared_ptr<RmlModel_MarkerComponent>;

class RmlModel_MarkerObjectSystem;
using RmlModel_MarkerObjectSystemPtr = std::shared_ptr<RmlModel_MarkerObjectSystem>;

class RmlModel_MarkerTrackingVolumeComponent;
using RmlModel_MarkerTrackingVolumeComponentPtr = std::shared_ptr<RmlModel_MarkerTrackingVolumeComponent>;

class RmlModel_MikanComponent;
using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;

class RmlModel_ModelStencilComponent;
using RmlModel_ModelStencilComponentPtr = std::shared_ptr<RmlModel_ModelStencilComponent>;

class RmlModel_NetworkVideoSourceComponent;
using RmlModel_NetworkVideoSourceComponentPtr = std::shared_ptr<RmlModel_NetworkVideoSourceComponent>;

class RmlModel_EntityAccessor;
using RmlModel_EntityAccessorPtr = std::shared_ptr<RmlModel_EntityAccessor>;

class RmlModel_QuadStencilComponent;
using RmlModel_QuadStencilComponentPtr = std::shared_ptr<RmlModel_QuadStencilComponent>;

class RmlModel_SceneComponent;
using RmlModel_SceneComponentPtr = std::shared_ptr<RmlModel_SceneComponent>;

class RmlModel_SpoutTextureSourceComponent;
using RmlModel_SpoutTextureSourceComponentPtr = std::shared_ptr<RmlModel_SpoutTextureSourceComponent>;

class RmlModel_StageComponent;
using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;

class RmlModel_TrackingMountComponent;
using RmlModel_TrackingMountComponentPtr = std::shared_ptr<RmlModel_TrackingMountComponent>;

class RmlModel_USBVideoSourceComponent;
using RmlModel_USBVideoSourceComponentPtr = std::shared_ptr<RmlModel_USBVideoSourceComponent>;

class RmlModel_VRTrackingVolumeComponent;
using RmlModel_VRTrackingVolumeComponentPtr = std::shared_ptr<RmlModel_VRTrackingVolumeComponent>;
