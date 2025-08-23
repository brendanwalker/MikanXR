#pragma once

#include <memory>

class SceneComponentDefinition;
using SceneComponentDefinitionPtr = std::shared_ptr<SceneComponentDefinition>;
using SceneComponentDefinitionConstPtr = std::shared_ptr<const SceneComponentDefinition>;
using SceneComponentDefinitionWeakPtr = std::weak_ptr<SceneComponentDefinition>;

class SceneComponent;
using SceneComponentPtr = std::shared_ptr<SceneComponent>;
using SceneComponentConstPtr = std::shared_ptr<const SceneComponent>;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;

class StageComponentDefinition;
using StageComponentDefinitionPtr = std::shared_ptr<StageComponentDefinition>;
using StageComponentDefinitionConstPtr = std::shared_ptr<const StageComponentDefinition>;
using StageComponentDefinitionWeakPtr = std::weak_ptr<StageComponentDefinition>;

class StageComponent;
using StageComponentPtr = std::shared_ptr<StageComponent>;
using StageComponentConstPtr = std::shared_ptr<const StageComponent>;
using StageComponentWeakPtr = std::weak_ptr<StageComponent>;

class TransformComponentDefinition;
using TransformComponentDefinitionPtr = std::shared_ptr<TransformComponentDefinition>;
using TransformComponentDefinitionConstPtr = std::shared_ptr<const TransformComponentDefinition>;
using TransformComponentDefinitionWeakPtr = std::weak_ptr<TransformComponentDefinition>;

class TransformComponent;
using TransformComponentPtr = std::shared_ptr<TransformComponent>;
using TransformComponentConstPtr = std::shared_ptr<const TransformComponent>;
using TransformComponentWeakPtr = std::weak_ptr<TransformComponent>;