#pragma once

#include <memory>
#include <string>

template <typename t_element_type> class RmlDataBinding_List;

using RmlDataBinding_IntList = RmlDataBinding_List<int>;
using RmlDataBinding_IntListPtr = std::shared_ptr<RmlDataBinding_IntList>;

using RmlDataBinding_ComponentIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ComponentIdListPtr = std::shared_ptr<RmlDataBinding_ComponentIdList>;

using RmlDataBinding_ScriptTriggerList = RmlDataBinding_List<std::string>;
using RmlDataBinding_ScriptTriggerListPtr = std::shared_ptr<RmlDataBinding_ScriptTriggerList>;

using RmlDataBinding_VRDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VRDevicePathListPtr = std::shared_ptr<RmlDataBinding_VRDevicePathList>;

using RmlDataBinding_SocketNameList = RmlDataBinding_List<std::string>;
using RmlDataBinding_SocketNameListPtr = std::shared_ptr<RmlDataBinding_SocketNameList>;

using RmlDataBinding_SpoutSourceList = RmlDataBinding_List<std::string>;
using RmlDataBinding_SpoutSourceListPtr = std::shared_ptr<RmlDataBinding_SpoutSourceList>;

using RmlDataBinding_USBDevicePathList = RmlDataBinding_List<std::string>;
using RmlDataBinding_USBDevicePathListPtr = std::shared_ptr<RmlDataBinding_USBDevicePathList>;

using RmlDataBinding_VideoResolutionList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VideoResolutionListPtr = std::shared_ptr<RmlDataBinding_VideoResolutionList>;

using RmlDataBinding_VideoFrameRateList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VideoFrameRateListPtr = std::shared_ptr<RmlDataBinding_VideoFrameRateList>;

using RmlDataBinding_VideoFormatList = RmlDataBinding_List<std::string>;
using RmlDataBinding_VideoFormatListPtr = std::shared_ptr<RmlDataBinding_VideoFormatList>;

class RmlDataBinding_VideoSourceSetting;
using RmlDataBinding_VideoSourceSettingPtr = std::shared_ptr<RmlDataBinding_VideoSourceSetting>;

class RmlModel_VideoSourceSettings;
using RmlModel_VideoSourceSettingsPtr = std::shared_ptr<RmlModel_VideoSourceSettings>;

class RmlModel_ClientTextureSourceComponent;
using RmlModel_ClientTextureSourceComponentPtr = std::shared_ptr<RmlModel_ClientTextureSourceComponent>;

class RmlModel_MikanComponent;
using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;

class RmlModel_NetworkVideoSourceComponent;
using RmlModel_NetworkVideoSourceComponentPtr = std::shared_ptr<RmlModel_NetworkVideoSourceComponent>;

class RmlModel_EntityAccessor;
using RmlModel_EntityAccessorPtr = std::shared_ptr<RmlModel_EntityAccessor>;

class RmlModel_SpoutTextureSourceComponent;
using RmlModel_SpoutTextureSourceComponentPtr = std::shared_ptr<RmlModel_SpoutTextureSourceComponent>;

class RmlModel_USBVideoSourceComponent;
using RmlModel_USBVideoSourceComponentPtr = std::shared_ptr<RmlModel_USBVideoSourceComponent>;
