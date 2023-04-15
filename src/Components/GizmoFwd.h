#pragma once

#include <memory>

// Gizmo Components
class GizmoTransformComponent;
using GizmoTransformComponentPtr = std::shared_ptr<GizmoTransformComponent>;
using GizmoTransformComponentWeakPtr = std::weak_ptr<GizmoTransformComponent>;

class GizmoTranslateComponent;
using GizmoTranslateComponentPtr = std::shared_ptr<GizmoTranslateComponent>;
using GizmoTranslateComponentWeakPtr = std::weak_ptr<GizmoTranslateComponent>;

class GizmoRotateComponent;
using GizmoRotateComponentPtr = std::shared_ptr<GizmoRotateComponent>;
using GizmoRotateComponentWeakPtr = std::weak_ptr<GizmoRotateComponent>;

class GizmoScaleComponent;
using GizmoScaleComponentPtr = std::shared_ptr<GizmoScaleComponent>;
using GizmoScaleComponentWeakPtr = std::weak_ptr<GizmoScaleComponent>;