#pragma once

#include <memory>

class ProjectConfig;
using ProjectConfigPtr = std::shared_ptr<ProjectConfig>;
using ProjectConfigConstPtr = std::shared_ptr<const ProjectConfig>;
using ProjectConfigWeakPtr = std::weak_ptr<ProjectConfig>;

class AnchorObjectSystemConfig;
using AnchorObjectSystemConfigPtr = std::shared_ptr<AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigConstPtr = std::shared_ptr<const AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigWeakPtr = std::weak_ptr<AnchorObjectSystemConfig>;

class CameraObjectSystemConfig;
using CameraObjectSystemConfigPtr = std::shared_ptr<CameraObjectSystemConfig>;
using CameraObjectSystemConfigConstPtr = std::shared_ptr<const CameraObjectSystemConfig>;
using CameraObjectSystemConfigWeakPtr = std::weak_ptr<CameraObjectSystemConfig>;

class EditorObjectSystemConfig;
using EditorObjectSystemConfigPtr = std::shared_ptr<EditorObjectSystemConfig>;
using EditorObjectSystemConfigConstPtr = std::shared_ptr<const EditorObjectSystemConfig>;
using EditorObjectSystemConfigWeakPtr = std::weak_ptr<EditorObjectSystemConfig>;

class SceneObjectSystemConfig;
using SceneObjectSystemConfigPtr = std::shared_ptr<SceneObjectSystemConfig>;
using SceneObjectSystemConfigConstPtr = std::shared_ptr<const SceneObjectSystemConfig>;
using SceneObjectSystemConfigWeakPtr = std::weak_ptr<SceneObjectSystemConfig>;

class StageObjectSystemConfig;
using StageObjectSystemConfigPtr = std::shared_ptr<StageObjectSystemConfig>;
using StageObjectSystemConfigConstPtr = std::shared_ptr<const StageObjectSystemConfig>;
using StageObjectSystemConfigWeakPtr = std::weak_ptr<StageObjectSystemConfig>;

class StencilObjectSystemConfig;
using StencilObjectSystemConfigPtr = std::shared_ptr<StencilObjectSystemConfig>;
using StencilObjectSystemConfigConstPtr = std::shared_ptr<const StencilObjectSystemConfig>;
using StencilObjectSystemConfigWeakPtr = std::weak_ptr<StencilObjectSystemConfig>;