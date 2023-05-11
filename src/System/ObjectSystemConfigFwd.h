#pragma once

#include <memory>

class ProfileConfig;
using ProfileConfigPtr = std::shared_ptr<ProfileConfig>;
using ProfileConfigConstPtr = std::shared_ptr<const ProfileConfig>;
using ProfileConfigWeakPtr = std::weak_ptr<ProfileConfig>;

class AnchorConfig;
using AnchorConfigPtr = std::shared_ptr<AnchorConfig>;
using AnchorConfigConstPtr = std::shared_ptr<const AnchorConfig>;
using AnchorConfigWeakPtr = std::weak_ptr<AnchorConfig>;

class AnchorObjectSystemConfig;
using AnchorObjectSystemConfigPtr = std::shared_ptr<AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigConstPtr = std::shared_ptr<const AnchorObjectSystemConfig>;
using AnchorObjectSystemConfigWeakPtr = std::weak_ptr<AnchorObjectSystemConfig>;

class EditorObjectSystemConfig;
using EditorObjectSystemConfigPtr = std::shared_ptr<EditorObjectSystemConfig>;
using EditorObjectSystemConfigConstPtr = std::shared_ptr<const EditorObjectSystemConfig>;
using EditorObjectSystemConfigWeakPtr = std::weak_ptr<EditorObjectSystemConfig>;

class FastenerConfig;
using FastenerConfigPtr = std::shared_ptr<FastenerConfig>;
using FastenerConfigConstPtr = std::shared_ptr<const FastenerConfig>;
using FastenerConfigWeakPtr = std::weak_ptr<FastenerConfig>;

class FastenerObjectSystemConfig;
using FastenerObjectSystemConfigPtr = std::shared_ptr<FastenerObjectSystemConfig>;
using FastenerObjectSystemConfigConstPtr = std::shared_ptr<const FastenerObjectSystemConfig>;
using FastenerObjectSystemConfigWeakPtr = std::weak_ptr<FastenerObjectSystemConfig>;

class QuadStencilConfig;
using QuadStencilConfigPtr = std::shared_ptr<QuadStencilConfig>;
using QuadStencilConfigConstPtr = std::shared_ptr<const QuadStencilConfig>;
using QuadStencilConfigWeakPtr = std::weak_ptr<QuadStencilConfig>;

class BoxStencilConfig;
using BoxStencilConfigPtr = std::shared_ptr<BoxStencilConfig>;
using BoxStencilConfigConstPtr = std::shared_ptr<const BoxStencilConfig>;
using BoxStencilConfigWeakPtr = std::weak_ptr<BoxStencilConfig>;

class ModelStencilConfig;
using ModelStencilConfigPtr = std::shared_ptr<ModelStencilConfig>;
using ModelStencilConfigConstPtr = std::shared_ptr<const ModelStencilConfig>;
using ModelStencilConfigWeakPtr = std::weak_ptr<ModelStencilConfig>;

class StencilObjectSystemConfig;
using StencilObjectSystemConfigPtr = std::shared_ptr<StencilObjectSystemConfig>;
using StencilObjectSystemConfigConstPtr = std::shared_ptr<const StencilObjectSystemConfig>;
using StencilObjectSystemConfigWeakPtr = std::weak_ptr<StencilObjectSystemConfig>;