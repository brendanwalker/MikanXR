# Objects

The editor's scene object system: everything under `src/Editor/ECS`. Objects are component containers, components pair with Configuru-backed definition configs, and per-type "object systems" own pools of them under a single `ProjectManager`. This doc covers the core types, lifecycle, transforms, persistence, IDs, the property/function surface that makes objects remotely controllable, and selection/gizmos. See [modules.md](./modules.md) for where `MikanEditor` sits in the build, [wire-protocol.md](./wire-protocol.md) for the client-facing property contract, and [scripting.md](./scripting.md) for the per-component Lua scripting hooks.

---

## Core types

- `MikanObject` (`src/Editor/ECS/MikanObject.h`) is a named component container. It holds a `std::vector<MikanComponentPtr>`, a weak pointer to its owning `MikanObjectSystem`, and a root `TransformComponent`. Components are added with `addComponent<T>()` and found with `getComponentOfType<T>()` / `getComponentOfTypeAndName<T>()` (dynamic-cast based, no type registry). Lifecycle is `init()`, `postInit()`, `dispose()`, called by the owning system.
- `MikanComponent` (`src/Editor/ECS/MikanComponent.h`) is the component base. It implements `IEntityAccessor` (see below), owns a `MikanComponentDefinitionPtr`, exposes `getComponentClassName()` via a per-class `k_componentClassName` string constant, and can opt into per-frame `update(float deltaSeconds)` by setting `m_bWantsUpdate` in its constructor. `customRender(...)` is invoked by the owning system for debug drawing. Each component can also carry a Lua `ComponentScriptContext`.
- `MikanComponentDefinition` is a `CommonConfig` subclass (`src/Editor/Config/CommonConfig.h`) holding the persisted state for a component: `m_componentId`, `m_componentName`, and an optional script asset reference. It implements `writeToJSON()` / `readFromJSON()` (Configuru) and `readFromInitParams()` (initialization from a client-supplied `Serialization::PolymorphicObjectPtr`, the wire-protocol creation path).
- `MikanObjectSystem` (`src/Editor/ECS/MikanObjectSystem.h`) owns a `MikanObjectList` and a `MikanObjectSystemDefinition` (also a `CommonConfig`). Systems are created and ticked by `ProjectManager` (`src/Editor/Project/ProjectManager.h`), which also owns the `MikanPropertyDatabase` and `MikanFunctionDatabase`. Systems find each other with `getObjectSystemOfType<T>()`.

The design rule throughout: the definition (config) is the persistent, change-notified source of truth; the component is the runtime view of it. `CommonConfig` provides `OnPropertyChanged` with a `ConfigPropertyChangeSet`, child-config nesting via `addChildConfig()`, autosave (`updateAutoSave`), and a `wantsSaveForPropertyChange()` veto (used by `CameraDefinition` to avoid saving every frame while a tracking mount drives the camera transform).

---

## Typed object systems and object creation

Most systems derive from the template `MikanTypedObjectSystem<TComponent, TDefinition, TID, TSystem, TSystemDefinition>` (`src/Editor/ECS/MikanTypedObjectSystem.h`): one system manages one pool (`MikanTypedComponentPool`) of one primary component type keyed by a typed ID (e.g. `AnchorObjectSystem : MikanTypedObjectSystem<AnchorComponent, AnchorDefinition, MikanSpatialAnchorID, AnchorObjectSystem, AnchorObjectSystemDefinition>`). The matching `MikanTypedObjectSystemDefinition<TComponent, TDefinition, TID>` holds a `MikanTypedComponentPoolDefinition` (added as a child config) that serializes the list of component definitions.

Object construction is centralized in `MikanTypedObjectSystem::objectFactory`:

1. `newEmptyObject()` creates the `MikanObject`.
2. The primary component is added and bound to its definition (`setDefinition`).
3. If the primary component is a `TransformComponent`, it becomes the object's root component.
4. `additionalComponentFactory(...)` lets the derived system add secondary components (e.g. `AnchorObjectSystem` adds a `SelectionComponent` and a `BoxColliderComponent` attached to the root).
5. `MikanObject::init()`, then `postInit()` if the system is already initialized.
6. The definition is added to the system definition's pool (which fires the config property-change event), and `OnNewObjectFinalized` is broadcast.

Creation entry points: `addNewObjectByTypedDefinition(initFunc)` in C++, and `addNewObjectByUntypedDefinition(primaryComponentClass, initParams)` for wire-protocol creation (matches on `TComponent::k_componentClassName`, then calls `TDefinition::readFromInitParams`). Loading a project calls `init()` on each system, which runs `m_componentPool.initializeFromDefinitions(...)` over the saved definitions. Deletion (`removeObjectByPrimaryComponentId`) is three steps in order: dispose the object (fires `OnObjectDisposed` / `OnComponentDisposed`), remove the component from the pool, then remove the definition from the system definition.

---

## The object systems that exist

`ProjectManager::startup` (`src/Editor/Project/ProjectManager.cpp`) registers, in order: `EditorObjectSystem`, `ClientTextureSourceSystem`, `SpoutTextureSourceSystem`, `CEFTextureSourceSystem`, `NetworkVideoSourceSystem`, `USBVideoSourceSystem`, `ARKitVideoSourceSystem`, `MarkerObjectSystem`, `StageObjectSystem`, `SceneObjectSystem`, `CompositorObjectSystem`, `CameraObjectSystem`, `AnchorObjectSystem`, `QuadShapeSystem`, `BoxShapeSystem`, `ModelShapeSystem`, `QuadStencilSystem`, `BoxStencilSystem`, `ModelStencilSystem`, `VRObjectSystem`, `TrackingMountObjectSystem`, `MarkerTrackingVolumeSystem`, `VRTrackingVolumeSystem`, `DMXObjectSystem`, `RGBSpotLightSystem`, `RGBPixelGridSystem`.

Primary component families (each with a matching `*Definition` config class):

- Scene structure: `SceneComponent` and `StageComponent` (both extend `TransformComponent`; a scene attaches to a stage via `attachToStage`, a stage references a tracking volume and stores stage bounds).
- Spatial: `AnchorComponent`, `MarkerComponent` (ArUco id + physical length in mm), `TrackingMountComponent` (binds a VR device by path/socket), `TrackingVolumeComponent` with `VRTrackingVolumeComponent` / `MarkerTrackingVolumeComponent` subclasses, `VRDeviceComponent` (runtime-only, under `VRObjectSystem`).
- Camera/video: `CameraComponent` (stage id, tracking mount id, video source id, aperture offset), `VideoSourceComponent` with `USBVideoSourceComponent` / `NetworkVideoSourceComponent` / `ARKitVideoSourceComponent` subclasses (see [videosources.md](./videosources.md)).
- Compositing inputs: `CompositorComponent`, `TextureSourceComponent` with `ClientTextureSourceComponent` / `SpoutTextureSourceComponent` / `CEFTextureSourceComponent` (see [compositor.md](./compositor.md)).
- Stencils and shapes: `StencilComponent` base with `QuadStencilComponent` / `BoxStencilComponent` / `ModelStencilComponent`; parallel `ShapeComponent` family `QuadShapeComponent` / `BoxShapeComponent` / `ModelShapeComponent`.
- Lighting: `DMXFixtureComponent`, `RGBSpotLightComponent`, `RGBPixelGridComponent`.
- Editor-only (secondary components, not system primaries): `SelectionComponent`, the gizmo components, `ColliderComponent` variants (`BoxColliderComponent`, `DiskColliderComponent`, `MeshColliderComponent` backed by `StaticMeshKdTree`), `StaticMeshComponent`.

Cross-system queries live in free-function headers: `ObjectSystemColliderQueries.h`, `ObjectSystemRenderQueries.h`, `TextureSourceQueries.h`, `TrackingVolumeQueries.h`, `VideoSourceQueries.h`.

---

## Component IDs

Component IDs are project-wide integers. `ProjectConfig` owns two allocators (`src/Editor/Project/ProjectConfig.h`): a `PersistentIDAllocator` whose counter is saved in the project file (so IDs never recycle across sessions) and a `MonotonicIDAllocator` for transient runtime-only components (e.g. `VRObjectSystem` devices) in a reserved high range (`k_transientIdStart`). The primary component's ID doubles as the domain ID: `getSceneId()`, `getCameraId()`, `getStageId()` etc. all return `getComponentId()`. `ProjectManager::getComponentById` resolves any ID by asking each system's pool.

---

## Transforms and parenting

`TransformComponent` (`src/Editor/ECS/Scene/TransformComponent.h`) holds a relative `GlmTransform` (TRS, `src/Libraries/MikanMath/Public/Transform.h`) and a cached world `glm::mat4`. Parenting is by component ID: `TransformComponentDefinition` persists `m_parentTransformId` (`k_parentTransformIdPropertyId`) alongside the relative scale/rotation/position. At runtime `attachToComponent(newParent)` / `detachFromParent(reason)` maintain parent/child weak-pointer lists, and `propogateWorldTransformChange` recomputes and pushes world transforms down the child list. `setWorldTransform` back-computes the relative transform from the parent. Math conventions (handedness, units, matrix order) are in [conventions.md](./conventions.md).

---

## Persistence

The project file is a single Configuru JSON document with extension `.mikanproj` (`ProjectManager::k_mikanProjectFileExtension`), written by `ProjectConfig::writeToJSON` via `ProjectManager::saveProject` / loaded by `loadProject`. `ProjectConfig` holds one `*ObjectSystemDefinition` per system (created through `addTypedDefinition<TConfig, TSystem>`, keyed by the system's `k_objectSystemClassName`), each of which serializes its component-definition pool under `k_componentPoolPropertyId`. So the persistence pattern per object type is: component class, definition (config) class, system definition holding the pool of definitions, all nested as `CommonConfig` children of `ProjectConfig`. Definition property setters call the change-notification helpers, which mark the config dirty and drive the autosave cooldown.

---

## Remote control and the property system

Every component and system implements `IEntityAccessor` (`src/Editor/ECS/IEntityAccessor.h`), which is `IPropertyInterface` + `IFunctionInterface`:

- Static `getPropertyDescriptors` / `getFunctionDescriptors` declare the schema (`PropertyDescriptor` with `MikanVariantType`, flags like `setReadOnly()`, `setUIHidden()`, `setClientAPIHidden()`).
- Virtual `getPropertyValue` / `setPropertyValue` (`MikanVariant`) and `invokeFunction` implement it.
- `getClientAPIValuesStructType()` returns the Refureku archetype of the matching client-API values struct.

`MikanTypedObjectSystem::registerPropertyDescriptors` registers both system and component schemas into the `ProjectManager`-owned `MikanPropertyDatabase` / `MikanFunctionDatabase`; each typed system also exposes its ID list via `k_componentIdListPropertyId` (served by `GetComponentListRequest`). The websocket side is `PropertyRequestHandler` / `FunctionRequestHandler` in `src/Editor/Server`; the values-struct/descriptor/`getPropertyValue` consistency is enforced by the schema-guard test run through `MikanCmd -runTests`. Details in [wire-protocol.md](./wire-protocol.md).

Note that `IRemoteControllable` / `RemoteControlManager` (`src/Editor/Server`) is a separate, string-command mechanism for remote-controlling `AppStage` UI screens (push/pop stage, `handleRemoteControlCommand`), not scene objects. Scene objects are remotely controlled through the property/function databases above.

---

## Selection, interaction, and gizmos

`EditorObjectSystem` (`src/Editor/ECS/Editor/EditorObjectSystem.h`) is the interaction hub. Viewports feed it mouse rays (`onMouseRayChanged`, `onMouseRayButtonDown/Up`); it raycasts against `ColliderComponent`s (`findClosestSelectionTarget`, `ColliderRaycastHitResult`) filtered by an object-system selection filter. Hit objects interact through their `SelectionComponent` (`src/Editor/ECS/Editor/SelectionComponent.h`), which broadcasts hover/grab/move/release/selected delegates; `setSelection` fires `OnSelectionChanged`. Selecting an object with a transform spawns a gizmo object (`createSceneTransformGizmo`) built from `GizmoTransformComponent` plus `GizmoTranslateComponent` / `GizmoRotateComponent` / `GizmoScaleComponent`, hit-tested through generated box/disk colliders. `EditorObjectSystem` also owns the orthographic ruler/measurement tool and the debug-draw flags, grid, and snap settings persisted in `EditorObjectSystemDefinition` (`EditorSettings`).
