#pragma once

#include <memory>

// Components
class MikanComponent;
using MikanComponentPtr= std::shared_ptr<MikanComponent>;
using MikanComponentWeakPtr= std::weak_ptr<MikanComponent>;

class AnchorComponent;
using AnchorComponentPtr = std::shared_ptr<AnchorComponent>;
using AnchorComponentWeakPtr = std::weak_ptr<AnchorComponent>;

class FastenerComponent;
using FastenerComponentPtr = std::shared_ptr<FastenerComponent>;
using FastenerComponentWeakPtr = std::weak_ptr<FastenerComponent>;

class SelectionComponent;
using SelectionComponentPtr = std::shared_ptr<SelectionComponent>;
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
class StencilComponent;
using StencilComponentPtr= std::shared_ptr<StencilComponent>;
using StencilComponentConstPtr= std::shared_ptr<const StencilComponent>;
using StencilComponentWeakPtr=  std::weak_ptr<StencilComponent>;

class QuadStencilComponent;
using QuadStencilComponentPtr = std::shared_ptr<QuadStencilComponent>;
using QuadStencilComponentConstPtr = std::shared_ptr<const QuadStencilComponent>;
using QuadStencilComponentWeakPtr = std::weak_ptr<QuadStencilComponent>;

class ModelStencilComponent;
using ModelStencilComponentPtr = std::shared_ptr<ModelStencilComponent>;
using ModelStencilComponentConstPtr = std::shared_ptr<const ModelStencilComponent>;
using ModelStencilComponentWeakPtr = std::weak_ptr<ModelStencilComponent>;

class BoxStencilComponent;
using BoxStencilComponentPtr = std::shared_ptr<BoxStencilComponent>;
using BoxStencilComponentConstPtr = std::shared_ptr<const BoxStencilComponent>;
using BoxStencilComponentWeakPtr = std::weak_ptr<BoxStencilComponent>;