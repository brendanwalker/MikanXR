#pragma once

#include <memory>

#include "VRDeviceFwd.h"

// Components
class MikanComponentDefinition;
using MikanComponentDefinitionPtr = std::shared_ptr<MikanComponentDefinition>;
using MikanComponentDefinitionConstPtr = std::shared_ptr<const MikanComponentDefinition>;
using MikanComponentDefinitionWeakPtr = std::weak_ptr<MikanComponentDefinition>;

class MikanComponent;
using MikanComponentPtr= std::shared_ptr<MikanComponent>;
using MikanComponentConstPtr= std::shared_ptr<const MikanComponent>;
using MikanComponentWeakPtr= std::weak_ptr<MikanComponent>;

class AnchorDefinition;
using AnchorDefinitionPtr = std::shared_ptr<AnchorDefinition>;
using AnchorDefinitionConstPtr = std::shared_ptr<const AnchorDefinition>;
using AnchorDefinitionWeakPtr = std::weak_ptr<AnchorDefinition>;

class AnchorComponent;
using AnchorComponentPtr = std::shared_ptr<AnchorComponent>;
using AnchorComponentWeakPtr = std::weak_ptr<AnchorComponent>;

class CameraDefinition;
using CameraDefinitionPtr = std::shared_ptr<CameraDefinition>;
using CameraDefinitionConstPtr = std::shared_ptr<const CameraDefinition>;
using CameraDefinitionWeakPtr = std::weak_ptr<CameraDefinition>;

class CameraComponent;
using CameraComponentPtr = std::shared_ptr<CameraComponent>;
using CameraComponentConstPtr = std::shared_ptr<const CameraComponent>;
using CameraComponentWeakPtr = std::weak_ptr<CameraComponent>;

class CompositorDefinition;
using CompositorDefinitionPtr = std::shared_ptr<CompositorDefinition>;
using CompositorDefinitionConstPtr = std::shared_ptr<const CompositorDefinition>;
using CompositorDefinitionWeakPtr = std::weak_ptr<CompositorDefinition>;

class CompositorComponent;
using CompositorComponentPtr = std::shared_ptr<CompositorComponent>;
using CompositorComponentWeakPtr = std::weak_ptr<CompositorComponent>;

class MarkerDefinition;
using MarkerDefinitionPtr = std::shared_ptr<MarkerDefinition>;
using MarkerDefinitionConstPtr = std::shared_ptr<const MarkerDefinition>;
using MarkerDefinitionWeakPtr = std::weak_ptr<MarkerDefinition>;

class MarkerComponent;
using MarkerComponentPtr = std::shared_ptr<MarkerComponent>;
using MarkerComponentConstPtr = std::shared_ptr<const MarkerComponent>;
using MarkerComponentWeakPtr = std::weak_ptr<MarkerComponent>;

class SelectionComponent;
using SelectionComponentPtr = std::shared_ptr<SelectionComponent>;
using SelectionComponentConstPtr = std::shared_ptr<const SelectionComponent>;
using SelectionComponentWeakPtr = std::weak_ptr<SelectionComponent>;

class TrackingVolumeDefinition;
using TrackingVolumeDefinitionPtr = std::shared_ptr<TrackingVolumeDefinition>;
using TrackingVolumeDefinitionConstPtr = std::shared_ptr<const TrackingVolumeDefinition>;
using TrackingVolumeDefinitionWeakPtr = std::weak_ptr<TrackingVolumeDefinition>;

class MarkerTrackingVolumeDefinition;
using MarkerTrackingVolumeDefinitionPtr = std::shared_ptr<MarkerTrackingVolumeDefinition>;
using MarkerTrackingVolumeDefinitionConstPtr = std::shared_ptr<const MarkerTrackingVolumeDefinition>;
using MarkerTrackingVolumeDefinitionWeakPtr = std::weak_ptr<MarkerTrackingVolumeDefinition>;

class VRTrackingVolumeDefinition;
using VRTrackingVolumeDefinitionPtr = std::shared_ptr<VRTrackingVolumeDefinition>;
using VRTrackingVolumeDefinitionConstPtr = std::shared_ptr<const VRTrackingVolumeDefinition>;
using VRTrackingVolumeDefinitionWeakPtr = std::weak_ptr<VRTrackingVolumeDefinition>;

class TrackingVolumeComponent;
using TrackingVolumeComponentPtr = std::shared_ptr<TrackingVolumeComponent>;
using TrackingVolumeComponentConstPtr = std::shared_ptr<const TrackingVolumeComponent>;
using TrackingVolumeComponentWeakPtr = std::weak_ptr<TrackingVolumeComponent>;

class MarkerTrackingVolumeComponent;
using MarkerTrackingVolumeComponentPtr = std::shared_ptr<MarkerTrackingVolumeComponent>;
using MarkerTrackingVolumeComponentConstPtr = std::shared_ptr<const MarkerTrackingVolumeComponent>;
using MarkerTrackingVolumeComponentWeakPtr = std::weak_ptr<MarkerTrackingVolumeComponent>;

class VRTrackingVolumeComponent;
using VRTrackingVolumeComponentPtr = std::shared_ptr<VRTrackingVolumeComponent>;
using VRTrackingVolumeComponentConstPtr = std::shared_ptr<const VRTrackingVolumeComponent>;
using VRTrackingVolumeComponentWeakPtr = std::weak_ptr<VRTrackingVolumeComponent>;

class TrackingMountDefinition;
using TrackingMountDefinitionPtr = std::shared_ptr<TrackingMountDefinition>;
using TrackingMountDefinitionConstPtr = std::shared_ptr<const TrackingMountDefinition>;
using TrackingMountDefinitionWeakPtr = std::weak_ptr<TrackingMountDefinition>;

class TrackingMountComponent;
using TrackingMountComponentPtr = std::shared_ptr<TrackingMountComponent>;
using TrackingMountComponentConstPtr = std::shared_ptr<const TrackingMountComponent>;
using TrackingMountComponentWeakPtr = std::weak_ptr<TrackingMountComponent>;

// Video Sources
class ClientVideoSourceDefinition;
using ClientVideoSourceDefinitionPtr = std::shared_ptr<ClientVideoSourceDefinition>;
using ClientVideoSourceDefinitionConstPtr = std::shared_ptr<const ClientVideoSourceDefinition>;
using ClientVideoSourceDefinitionWeakPtr = std::weak_ptr<ClientVideoSourceDefinition>;

class ClientVideoSourceComponent;
using ClientVideoSourceComponentPtr = std::shared_ptr<ClientVideoSourceComponent>;
using ClientVideoSourceComponentConstPtr = std::shared_ptr<const ClientVideoSourceComponent>;
using ClientVideoSourceComponentWeakPtr = std::weak_ptr<ClientVideoSourceComponent>;

class NetworkVideoSourceDefinition;
using NetworkVideoSourceDefinitionPtr = std::shared_ptr<NetworkVideoSourceDefinition>;
using NetworkVideoSourceDefinitionConstPtr = std::shared_ptr<const NetworkVideoSourceDefinition>;
using NetworkVideoSourceDefinitionWeakPtr = std::weak_ptr<NetworkVideoSourceDefinition>;

class NetworkVideoSourceComponent;
using NetworkVideoSourceComponentPtr = std::shared_ptr<NetworkVideoSourceComponent>;
using NetworkVideoSourceComponentConstPtr = std::shared_ptr<const NetworkVideoSourceComponent>;
using NetworkVideoSourceComponentWeakPtr = std::weak_ptr<NetworkVideoSourceComponent>;

class SpoutVideoSourceDefinition;
using SpoutVideoSourceDefinitionPtr = std::shared_ptr<SpoutVideoSourceDefinition>;
using SpoutVideoSourceDefinitionConstPtr = std::shared_ptr<const SpoutVideoSourceDefinition>;
using SpoutVideoSourceDefinitionWeakPtr = std::weak_ptr<SpoutVideoSourceDefinition>;

class SpoutVideoSourceComponent;
using SpoutVideoSourceComponentPtr = std::shared_ptr<SpoutVideoSourceComponent>;
using SpoutVideoSourceComponentConstPtr = std::shared_ptr<const SpoutVideoSourceComponent>;
using SpoutVideoSourceComponentWeakPtr = std::weak_ptr<SpoutVideoSourceComponent>;

class USBVideoSourceDefinition;
using USBVideoSourceDefinitionPtr = std::shared_ptr<USBVideoSourceDefinition>;
using USBVideoSourceDefinitionConstPtr = std::shared_ptr<const USBVideoSourceDefinition>;
using USBVideoSourceDefinitionWeakPtr = std::weak_ptr<USBVideoSourceDefinition>;

class USBVideoSourceComponent;
using USBVideoSourceComponentPtr = std::shared_ptr<USBVideoSourceComponent>;
using USBVideoSourceComponentConstPtr = std::shared_ptr<const USBVideoSourceComponent>;
using USBVideoSourceComponentWeakPtr = std::weak_ptr<USBVideoSourceComponent>;

class VideoSourceDefinition;
using VideoSourceDefinitionPtr = std::shared_ptr<VideoSourceDefinition>;
using VideoSourceDefinitionConstPtr = std::shared_ptr<const VideoSourceDefinition>;
using VideoSourceDefinitionWeakPtr = std::weak_ptr<VideoSourceDefinition>;

class VideoSourceComponent;
using VideoSourceComponentPtr = std::shared_ptr<VideoSourceComponent>;
using VideoSourceComponentConstPtr = std::shared_ptr<const VideoSourceComponent>;
using VideoSourceComponentWeakPtr = std::weak_ptr<VideoSourceComponent>;

// Mesh Components
class StaticMeshComponent;
using StaticMeshComponentPtr = std::shared_ptr<StaticMeshComponent>;
using StaticMeshComponentWeakPtr = std::weak_ptr<StaticMeshComponent>;

// Collider Components
class ColliderComponent;
using ColliderComponentWeakPtr = std::weak_ptr<ColliderComponent>;
using ColliderComponentPtr = std::shared_ptr<ColliderComponent>;

class BoxColliderComponent;
using BoxColliderComponentPtr = std::shared_ptr<BoxColliderComponent>;
using BoxColliderComponentWeakPtr = std::weak_ptr<BoxColliderComponent>;

class DiskColliderComponent;
using DiskColliderComponentPtr = std::shared_ptr<DiskColliderComponent>;
using DiskColliderComponentWeakPtr = std::weak_ptr<DiskColliderComponent>;

class MeshColliderComponent;
using MeshColliderComponentPtr= std::shared_ptr<MeshColliderComponent>;
using MeshColliderComponentWeakPtr= std::weak_ptr<MeshColliderComponent>;

// Stencil Components
class StencilComponentDefinition;
using StencilComponentConfigPtr = std::shared_ptr<StencilComponentDefinition>;
using StencilComponentConfigConstPtr = std::shared_ptr<const StencilComponentDefinition>;
using StencilComponentConfigWeakPtr = std::weak_ptr<StencilComponentDefinition>;

class StencilComponent;
using StencilComponentPtr= std::shared_ptr<StencilComponent>;
using StencilComponentConstPtr= std::shared_ptr<const StencilComponent>;
using StencilComponentWeakPtr=  std::weak_ptr<StencilComponent>;

class QuadStencilDefinition;
using QuadStencilDefinitionPtr = std::shared_ptr<QuadStencilDefinition>;
using QuadStencilDefinitionConstPtr = std::shared_ptr<const QuadStencilDefinition>;
using QuadStencilDefinitionWeakPtr = std::weak_ptr<QuadStencilDefinition>;

class QuadStencilComponent;
using QuadStencilComponentPtr = std::shared_ptr<QuadStencilComponent>;
using QuadStencilComponentConstPtr = std::shared_ptr<const QuadStencilComponent>;
using QuadStencilComponentWeakPtr = std::weak_ptr<QuadStencilComponent>;

class ModelStencilDefinition;
using ModelStencilDefinitionPtr = std::shared_ptr<ModelStencilDefinition>;
using ModelStencilDefinitionConstPtr = std::shared_ptr<const ModelStencilDefinition>;
using ModelStencilDefinitionWeakPtr = std::weak_ptr<ModelStencilDefinition>;

class ModelStencilComponent;
using ModelStencilComponentPtr = std::shared_ptr<ModelStencilComponent>;
using ModelStencilComponentConstPtr = std::shared_ptr<const ModelStencilComponent>;
using ModelStencilComponentWeakPtr = std::weak_ptr<ModelStencilComponent>;

class BoxStencilDefinition;
using BoxStencilDefinitionPtr = std::shared_ptr<BoxStencilDefinition>;
using BoxStencilDefinitionConstPtr = std::shared_ptr<const BoxStencilDefinition>;
using BoxStencilDefinitionWeakPtr = std::weak_ptr<BoxStencilDefinition>;

class BoxStencilComponent;
using BoxStencilComponentPtr = std::shared_ptr<BoxStencilComponent>;
using BoxStencilComponentConstPtr = std::shared_ptr<const BoxStencilComponent>;
using BoxStencilComponentWeakPtr = std::weak_ptr<BoxStencilComponent>;