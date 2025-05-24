#pragma once

#include <memory>

class AnchorObjectSystem;
using AnchorObjectSystemPtr = std::shared_ptr<AnchorObjectSystem>;
using AnchorObjectSystemWeakPtr = std::weak_ptr<AnchorObjectSystem>;

class CameraObjectSystem;
using CameraObjectSystemPtr = std::shared_ptr<CameraObjectSystem>;
using CameraObjectSystemWeakPtr = std::weak_ptr<CameraObjectSystem>;

class MikanObjectSystem;
using MikanObjectSystemPtr= std::shared_ptr<MikanObjectSystem>;
using MikanObjectSystemWeakPtr= std::weak_ptr<MikanObjectSystem>;

class EditorObjectSystem;
using EditorObjectSystemPtr = std::shared_ptr<EditorObjectSystem>;
using EditorObjectSystemWeakPtr = std::weak_ptr<EditorObjectSystem>;

class ObjectSystemManager;
using ObjectSystemManagerPtr = std::shared_ptr<ObjectSystemManager>;
using ObjectSystemManagerWeakPtr = std::weak_ptr<ObjectSystemManager>;

class SceneObjectSystem;
using SceneObjectSystemPtr = std::shared_ptr<SceneObjectSystem>;
using SceneObjectSystemWeakPtr = std::weak_ptr<SceneObjectSystem>;

class StageObjectSystem;
using StageObjectSystemPtr = std::shared_ptr<StageObjectSystem>;
using StageObjectSystemWeakPtr = std::weak_ptr<StageObjectSystem>;

class StencilObjectSystem;
using StencilObjectSystemPtr = std::shared_ptr<StencilObjectSystem>;
using StencilObjectSystemWeakPtr = std::weak_ptr<StencilObjectSystem>;