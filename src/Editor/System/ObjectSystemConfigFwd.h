#pragma once

#include <memory>

class ProjectConfig;
using ProfileConfigPtr = std::shared_ptr<ProjectConfig>;
using ProfileConfigConstPtr = std::shared_ptr<const ProjectConfig>;
using ProfileConfigWeakPtr = std::weak_ptr<ProjectConfig>;

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