#pragma once

#include <memory>

class AnchorObjectSystem;
using AnchorObjectSystemPtr = std::shared_ptr<AnchorObjectSystem>;
using AnchorObjectSystemWeakPtr = std::weak_ptr<AnchorObjectSystem>;

class CameraObjectSystem;
using CameraObjectSystemPtr = std::shared_ptr<CameraObjectSystem>;
using CameraObjectSystemWeakPtr = std::weak_ptr<CameraObjectSystem>;

class ClientTextureSourceSystem;
using ClientTextureSourceSystemPtr = std::shared_ptr<ClientTextureSourceSystem>;
using ClientTextureSourceSystemWeakPtr = std::weak_ptr<ClientTextureSourceSystem>;

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

class MarkerObjectSystem;
using MarkerObjectSystemPtr = std::shared_ptr<MarkerObjectSystem>;
using MarkerObjectSystemWeakPtr = std::weak_ptr<MarkerObjectSystem>;

class MarkerTrackingVolumeSystem;
using MarkerTrackingVolumeSystemPtr = std::shared_ptr<MarkerTrackingVolumeSystem>;
using MarkerTrackingVolumeSystemWeakPtr = std::weak_ptr<MarkerTrackingVolumeSystem>;

class NetworkVideoSourceSystem;
using NetworkVideoSourceSystemPtr = std::shared_ptr<NetworkVideoSourceSystem>;
using NetworkVideoSourceSystemWeakPtr = std::weak_ptr<NetworkVideoSourceSystem>;

class ProjectManager;
using ProjectManagerPtr = std::shared_ptr<ProjectManager>;
using ProjectManagerConstPtr = std::shared_ptr<const ProjectManager>;
using ProjectManagerWeakPtr = std::weak_ptr<ProjectManager>;

class SceneObjectSystem;
using SceneObjectSystemPtr = std::shared_ptr<SceneObjectSystem>;
using SceneObjectSystemWeakPtr = std::weak_ptr<SceneObjectSystem>;

class SpoutTextureSourceSystem;
using SpoutTextureSourceSystemPtr = std::shared_ptr<SpoutTextureSourceSystem>;
using SpoutTextureSourceSystemWeakPtr = std::weak_ptr<SpoutTextureSourceSystem>;

class CEFTextureSourceSystem;
using CEFTextureSourceSystemPtr = std::shared_ptr<CEFTextureSourceSystem>;
using CEFTextureSourceSystemWeakPtr = std::weak_ptr<CEFTextureSourceSystem>;

class StageObjectSystem;
using StageObjectSystemPtr = std::shared_ptr<StageObjectSystem>;
using StageObjectSystemWeakPtr = std::weak_ptr<StageObjectSystem>;

class BoxStencilSystem;
using BoxStencilSystemPtr = std::shared_ptr<BoxStencilSystem>;
using BoxStencilSystemWeakPtr = std::weak_ptr<BoxStencilSystem>;

class ModelStencilSystem;
using ModelStencilSystemPtr = std::shared_ptr<ModelStencilSystem>;
using ModelStencilSystemWeakPtr = std::weak_ptr<ModelStencilSystem>;

class QuadStencilSystem;
using QuadStencilSystemPtr = std::shared_ptr<QuadStencilSystem>;
using QuadStencilSystemWeakPtr = std::weak_ptr<QuadStencilSystem>;

class TrackingMountObjectSystem;
using TrackingMountObjectSystemPtr = std::shared_ptr<TrackingMountObjectSystem>;
using TrackingMountObjectSystemWeakPtr = std::weak_ptr<TrackingMountObjectSystem>;

class USBVideoSourceSystem;
using USBVideoSourceSystemPtr = std::shared_ptr<USBVideoSourceSystem>;
using USBVideoSourceSystemWeakPtr = std::weak_ptr<USBVideoSourceSystem>;

class VRObjectSystem;
using VRObjectSystemPtr = std::shared_ptr<VRObjectSystem>;
using VRObjectSystemWeakPtr = std::weak_ptr<VRObjectSystem>;

class VRTrackingVolumeSystem;
using VRTrackingVolumeSystemPtr = std::shared_ptr<VRTrackingVolumeSystem>;
using VRTrackingVolumeSystemWeakPtr = std::weak_ptr<VRTrackingVolumeSystem>;