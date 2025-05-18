#pragma once

#include <memory>

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
using CameraComponentWeakPtr = std::weak_ptr<CameraComponent>;

class SelectionComponent;
using SelectionComponentPtr = std::shared_ptr<SelectionComponent>;
using SelectionComponentConstPtr = std::shared_ptr<const SelectionComponent>;
using SelectionComponentWeakPtr = std::weak_ptr<SelectionComponent>;

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