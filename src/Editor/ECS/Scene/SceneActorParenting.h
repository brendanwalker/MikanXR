#pragma once

#include "SceneFwd.h"

// Anchors, stencils and shapes are the scene actors. They live under a scene, or
// nested under one another, and never directly under a stage or a camera. The
// three component types share one predicate rather than each spelling it out,
// since they share one rule.
bool isValidSceneActorParent(TransformComponentConstPtr parentComponent);
