#pragma once

#include <memory>

class AnchorObjectSystem;
using AnchorObjectSystemPtr = std::shared_ptr<AnchorObjectSystem>;
using AnchorObjectSystemWeakPtr = std::weak_ptr<AnchorObjectSystem>;

class FastenerObjectSystem;
using FastenerObjectSystemPtr = std::shared_ptr<FastenerObjectSystem>;
using FastenerObjectSystemWeakPtr = std::weak_ptr<FastenerObjectSystem>;

class MikanObjectSystem;
using MikanObjectSystemPtr= std::shared_ptr<MikanObjectSystem>;
using MikanObjectSystemWeakPtr= std::weak_ptr<MikanObjectSystem>;

class EditorObjectSystem;
using EditorObjectSystemPtr = std::shared_ptr<EditorObjectSystem>;
using EditorObjectSystemWeakPtr = std::weak_ptr<EditorObjectSystem>;

class ObjectSystemManager;
using ObjectSystemManagerPtr = std::shared_ptr<ObjectSystemManager>;
using ObjectSystemManagerWeakPtr = std::weak_ptr<ObjectSystemManager>;

class StencilObjectSystem;
using StencilObjectSystemPtr = std::shared_ptr<StencilObjectSystem>;
using StencilObjectSystemWeakPtr = std::weak_ptr<StencilObjectSystem>;