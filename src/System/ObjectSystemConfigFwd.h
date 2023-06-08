#pragma once

#include <memory>

class ProfileConfig;
using ProfileConfigPtr = std::shared_ptr<ProfileConfig>;
using ProfileConfigConstPtr = std::shared_ptr<const ProfileConfig>;
using ProfileConfigWeakPtr = std::weak_ptr<ProfileConfig>;

class AnchorObjectSystemConfig;
using AnchorObjectSystemConfigPtr = std::shared_ptr<AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigConstPtr = std::shared_ptr<const AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigWeakPtr = std::weak_ptr<AnchorObjectSystemConfig>;

class EditorObjectSystemConfig;
using EditorObjectSystemConfigPtr = std::shared_ptr<EditorObjectSystemConfig>;
using EditorObjectSystemConfigConstPtr = std::shared_ptr<const EditorObjectSystemConfig>;
using EditorObjectSystemConfigWeakPtr = std::weak_ptr<EditorObjectSystemConfig>;

class StencilObjectSystemConfig;
using StencilObjectSystemConfigPtr = std::shared_ptr<StencilObjectSystemConfig>;
using StencilObjectSystemConfigConstPtr = std::shared_ptr<const StencilObjectSystemConfig>;
using StencilObjectSystemConfigWeakPtr = std::weak_ptr<StencilObjectSystemConfig>;