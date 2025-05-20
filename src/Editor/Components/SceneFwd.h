#pragma once

#include <memory>

class MikanScene;
using MikanScenePtr = std::shared_ptr<MikanScene>;
using MikanSceneConstPtr = std::shared_ptr<const MikanScene>;
using MikanSceneWeakPtr = std::weak_ptr<MikanScene>;

class TransformComponentDefinition;
using TransformComponentDefinitionPtr = std::shared_ptr<TransformComponentDefinition>;
using TransformComponentDefinitionConstPtr = std::shared_ptr<const TransformComponentDefinition>;
using TransformComponentDefinitionWeakPtr = std::weak_ptr<TransformComponentDefinition>;

class TransformComponent;
using TransformComponentPtr = std::shared_ptr<TransformComponent>;
using TransformComponentConstPtr = std::shared_ptr<const TransformComponent>;
using TransformComponentWeakPtr = std::weak_ptr<TransformComponent>;