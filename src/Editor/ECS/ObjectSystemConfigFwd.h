#pragma once

#include <memory>

class AppSettingsConfig;
using AppSettingsConfigPtr = std::shared_ptr<AppSettingsConfig>;
using AppSettingsConfigConstPtr = std::shared_ptr<const AppSettingsConfig>;
using AppSettingsConfigWeakPtr = std::weak_ptr<AppSettingsConfig>;

class ProjectConfig;
using ProjectConfigPtr = std::shared_ptr<ProjectConfig>;
using ProjectConfigConstPtr = std::shared_ptr<const ProjectConfig>;
using ProjectConfigWeakPtr = std::weak_ptr<ProjectConfig>;

class MikanObjectSystemDefinition;
using MikanObjectSystemDefinitionPtr = std::shared_ptr<MikanObjectSystemDefinition>;
using MikanObjectSystemDefinitionConstPtr = std::shared_ptr<const MikanObjectSystemDefinition>;
using MikanObjectSystemDefinitionWeakPtr = std::weak_ptr<MikanObjectSystemDefinition>;

class AnchorObjectSystemConfig;
using AnchorObjectSystemConfigPtr = std::shared_ptr<AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigConstPtr = std::shared_ptr<const AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigWeakPtr = std::weak_ptr<AnchorObjectSystemConfig>;

class CameraObjectSystemConfig;
using CameraObjectSystemConfigPtr = std::shared_ptr<CameraObjectSystemConfig>;
using CameraObjectSystemConfigConstPtr = std::shared_ptr<const CameraObjectSystemConfig>;
using CameraObjectSystemConfigWeakPtr = std::weak_ptr<CameraObjectSystemConfig>;

class CompositorObjectSystemConfig;
using CompositorObjectSystemConfigPtr = std::shared_ptr<CompositorObjectSystemConfig>;
using CompositorObjectSystemConfigConstPtr = std::shared_ptr<const CompositorObjectSystemConfig>;
using CompositorObjectSystemConfigWeakPtr = std::weak_ptr<CompositorObjectSystemConfig>;

class EditorObjectSystemConfig;
using EditorObjectSystemConfigPtr = std::shared_ptr<EditorObjectSystemConfig>;
using EditorObjectSystemConfigConstPtr = std::shared_ptr<const EditorObjectSystemConfig>;
using EditorObjectSystemConfigWeakPtr = std::weak_ptr<EditorObjectSystemConfig>;

class MarkerSystemConfig;
using MarkerSystemConfigPtr = std::shared_ptr<MarkerSystemConfig>;
using MarkerSystemConfigConstPtr = std::shared_ptr<const MarkerSystemConfig>;
using MarkerSystemConfigWeakPtr = std::weak_ptr<MarkerSystemConfig>;

class MarkerObjectSystemConfig;
using MarkerObjectSystemConfigPtr = std::shared_ptr<MarkerObjectSystemConfig>;
using MarkerObjectSystemConfigConstPtr = std::shared_ptr<const MarkerObjectSystemConfig>;
using MarkerObjectSystemConfigWeakPtr = std::weak_ptr<MarkerObjectSystemConfig>;

class NetworkVideoSourceSystemConfig;
using NetworkVideoSourceSystemConfigPtr = std::shared_ptr<NetworkVideoSourceSystemConfig>;
using NetworkVideoSourceSystemConfigConstPtr = std::shared_ptr<const NetworkVideoSourceSystemConfig>;
using NetworkVideoSourceSystemConfigWeakPtr = std::weak_ptr<NetworkVideoSourceSystemConfig>;

class SceneObjectSystemDefinition;
using SceneObjectSystemDefinitionPtr = std::shared_ptr<SceneObjectSystemDefinition>;
using SceneObjectSystemDefinitionConstPtr = std::shared_ptr<const SceneObjectSystemDefinition>;
using SceneObjectSystemDefinitionWeakPtr = std::weak_ptr<SceneObjectSystemDefinition>;

class StageObjectSystemConfig;
using StageObjectSystemConfigPtr = std::shared_ptr<StageObjectSystemConfig>;
using StageObjectSystemConfigConstPtr = std::shared_ptr<const StageObjectSystemConfig>;
using StageObjectSystemConfigWeakPtr = std::weak_ptr<StageObjectSystemConfig>;

class StencilObjectSystemConfig;
using StencilObjectSystemConfigPtr = std::shared_ptr<StencilObjectSystemConfig>;
using StencilObjectSystemConfigConstPtr = std::shared_ptr<const StencilObjectSystemConfig>;
using StencilObjectSystemConfigWeakPtr = std::weak_ptr<StencilObjectSystemConfig>;

class TrackingMountObjectSystemConfig;
using TrackingMountObjectSystemConfigPtr = std::shared_ptr<TrackingMountObjectSystemConfig>;
using TrackingMountObjectSystemConfigConstPtr = std::shared_ptr<const TrackingMountObjectSystemConfig>;
using TrackingMountObjectSystemConfigWeakPtr = std::weak_ptr<TrackingMountObjectSystemConfig>;

class TrackingVolumeObjectSystemConfig;
using TrackingVolumeObjectSystemConfigPtr = std::shared_ptr<TrackingVolumeObjectSystemConfig>;
using TrackingVolumeObjectSystemConfigConstPtr = std::shared_ptr<const TrackingVolumeObjectSystemConfig>;
using TrackingVolumeObjectSystemConfigWeakPtr = std::weak_ptr<TrackingVolumeObjectSystemConfig>;

class TextureSourceSystemConfig;
using TextureSourceSystemConfigPtr = std::shared_ptr<TextureSourceSystemConfig>;
using TextureSourceSystemConfigConstPtr = std::shared_ptr<const TextureSourceSystemConfig>;
using TextureSourceSystemConfigWeakPtr = std::weak_ptr<TextureSourceSystemConfig>;

class USBVideoSourceSystemConfig;
using USBVideoSourceSystemConfigPtr = std::shared_ptr<USBVideoSourceSystemConfig>;
using USBVideoSourceSystemConfigConstPtr = std::shared_ptr<const USBVideoSourceSystemConfig>;
using USBVideoSourceSystemConfigWeakPtr = std::weak_ptr<USBVideoSourceSystemConfig>;

class VRObjectSystemConfig;
using VRObjectSystemConfigPtr = std::shared_ptr<VRObjectSystemConfig>;
using VRObjectSystemConfigConstPtr = std::shared_ptr<const VRObjectSystemConfig>;
using VRObjectSystemConfigWeakPtr = std::weak_ptr<VRObjectSystemConfig>;