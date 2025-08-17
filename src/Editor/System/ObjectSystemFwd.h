#pragma once

#include <memory>

class AnchorObjectSystem;
using AnchorObjectSystemPtr = std::shared_ptr<AnchorObjectSystem>;
using AnchorObjectSystemWeakPtr = std::weak_ptr<AnchorObjectSystem>;

class CameraObjectSystem;
using CameraObjectSystemPtr = std::shared_ptr<CameraObjectSystem>;
using CameraObjectSystemWeakPtr = std::weak_ptr<CameraObjectSystem>;

class CompositorObjectSystem;
using CompositorObjectSystemPtr = std::shared_ptr<CompositorObjectSystem>;
using CompositorObjectSystemWeakPtr = std::weak_ptr<CompositorObjectSystem>;

class MikanObjectSystem;
using MikanObjectSystemPtr= std::shared_ptr<MikanObjectSystem>;
using MikanObjectSystemWeakPtr= std::weak_ptr<MikanObjectSystem>;

class EditorObjectSystem;
using EditorObjectSystemPtr = std::shared_ptr<EditorObjectSystem>;
using EditorObjectSystemWeakPtr = std::weak_ptr<EditorObjectSystem>;

class MarkerSystem;
using MarkerSystemPtr = std::shared_ptr<MarkerSystem>;
using MarkerSystemWeakPtr = std::weak_ptr<MarkerSystem>;

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

class VideoSourceSystem;
using VideoSourceSystemPtr = std::shared_ptr<VideoSourceSystem>;
using VideoSourceSystemWeakPtr = std::weak_ptr<VideoSourceSystem>;

class VRObjectSystem;
using VRObjectSystemPtr = std::shared_ptr<VRObjectSystem>;
using VRObjectSystemWeakPtr = std::weak_ptr<VRObjectSystem>;