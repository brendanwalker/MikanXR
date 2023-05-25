#pragma once

#include <memory>

class MikanScene;
using MikanScenePtr = std::shared_ptr<MikanScene>;
using MikanSceneConstPtr = std::shared_ptr<const MikanScene>;
using MikanSceneWeakPtr = std::weak_ptr<MikanScene>;

class SceneComponent;
using SceneComponentPtr = std::shared_ptr<SceneComponent>;
using SceneComponentConstPtr = std::shared_ptr<const SceneComponent>;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;