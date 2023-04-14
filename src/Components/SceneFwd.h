#pragma once

#include <memory>

class MikanScene;
using MikanScenePtr = std::shared_ptr<MikanScene>;
using MikanSceneWeakPtr = std::weak_ptr<MikanScene>;

class SceneComponent;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;
using SceneComponentPtr = std::shared_ptr<SceneComponent>;
