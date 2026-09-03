# Scripting

MikanXR has two programmable layers: visual node graphs (`src/Editor/NodeEditors/`) that drive rendering pipelines, and project-level Lua scripts (`src/Editor/Scripting/`) that drive scene logic. The compositor graph's rendering semantics are covered in [compositor.md](./compositor.md), the ECS objects scripts manipulate in [objects.md](./objects.md), and the client-facing script RPC in [wire-protocol.md](./wire-protocol.md).

---

## Graph types

Two `NodeGraph` subclasses exist, registered in `App::startup` via `NodeGraphFactory::registerFactory<>`:

- `CompositorNodeGraph` (`Graphs/CompositorNodeGraph.h`): composites a video frame. Entry point is the `EventNode` named `OnCompositeFrame` (`k_compositeFrameEventName`); evaluated once per composited frame by `CompositorComponent`.

- `ShapeNodeGraph` (`Graphs/ShapeNodeGraph.h`): renders a shape component. Entry point is the `OnRenderShape` event (`k_renderShapeEventName`); `renderShape()` sets a transient view-projection matrix, evaluates the chain, then clears it. Bound to a `ShapeComponent`.

There is no general-purpose logic graph; non-rendering logic is done in Lua.

---

## Node / pin / link model

All types live under `src/Editor/NodeEditors/`:

- `NodeGraph` owns maps of `Node`, `NodePin`, and `NodeLink` by integer id (`allocateId()`), plus graph-level `GraphProperty` objects and `AssetReference` entries. Each graph subclass declares which node, pin, property, and asset-reference factories it supports (`addNodeFactory<>`, `addPinFactory<>`, ...).

- `Node` (`Nodes/Node.h`) has input/output pin lists, `evaluateNode(NodeEvaluator&)`, and editor hooks (`editorRenderNode`, `editorRenderPropertySheet`). Factories (`TypedNodeFactory<Node, NodeConfig>`) create nodes with their default pins.

- `NodePin` (`Pins/NodePin.h`) has a direction, connected `NodeLink` list, an optional default value (`setHasDefaultValue`, lets a node evaluate with the pin unconnected), and a dynamic flag (`setIsDynamicPin`, for pins generated from another pin's value, as when `DrawLayerNode` creates one pin per shader uniform of its bound material). Concrete pins: `FlowPin`, `FloatPin`/`Float2Pin`/`Float3Pin`/`Float4Pin`, `IntPin`, `BoolPin`, `TexturePin`, `PropertyPin`, `ArrayPin` (typed by element property class).

- `NodeLink` (`Pins/NodeLink.h`) connects one output pin to one input pin.

- Graph properties (`Properties/`) wrap referenced resources as graph-level values: `GraphMaterialProperty`, `GraphTextureProperty`, `GraphStencilProperty`, `GraphShapeProperty`, `GraphModelProperty`, `GraphBoolProperty`, array/value variants. Each carries a display name (editable in the editor's Details panel, and defaulted to the dropped asset's name when one is dragged in) plus a sort order setting its place in the Variables list, both persisted in the graph file. Names are display-only: nodes and pins reference properties by id, so renaming never breaks a link.

The editor UI is Dear ImGui plus imgui-node-editor, reached through the `MkCanvas` facade in `MikanGUI` (see [modules.md](./modules.md)). The canvas pans and wheel-zooms with text kept crisp at any zoom, and link drags give live accept/reject feedback from `NodePin::canPinsBeConnected`. Editor windows are `NodeEditorWindow` subclasses in `Windows/`: `CompositorNodeEditorWindow` (opened by `CompositorComponent::editCompositorGraph()`), `ShapeNodeEditorWindow`, and `CompositorOutputEditorWindow` (output preview, not a graph editor). The graph editors use the same dockable panel shell as the main window (Graph, Variables, Assets, and Details panels under a File/Edit/View menu bar, layout persisted per window). Graph edits are undoable through a per-window snapshot history ([transactions.md](./transactions.md)).

---

## Evaluation

`NodeEvaluator` (`Graphs/NodeEvaluator.h`) carries the graphics context and delta time. `evaluateFlowPinChain(startNode)` evaluates the start node, then follows the first link off each node's output `FlowPin` to the next node, stopping on a node with no output flow pin, on an error, or after 1000 nodes (`kInifiniteLoopThreshold`, reported as an infinite-loop error). Non-flow input pins are pulled on demand by the node being evaluated (`evaluateInputs`). Failures accumulate as `NodeEvaluationError` values on the evaluator; the owning component stores them (`getLastNodeEvalErrors()`) and the node editor windows display them.

---

## Graph persistence

Graphs are standalone assets, not part of the project config. `NodeGraphFactory::saveNodeGraph`/`loadNodeGraph` round-trip a `NodeGraphConfig` (Configuru JSON) holding the graph class name, id counter, and per-object configs for asset refs, properties, nodes, pins, and links. Files use the `*.graph` extension; `NodeGraphAssetReferenceFactory` defaults to the project's `graphs/` directory. Loading dispatches on the stored class name to the registered factory, allocates the graph, then rebuilds objects through the graph's own factories. Components reference graphs by path through a `NodeGraphAssetReference` stored in their definition (e.g. `CompositorDefinition::k_compositorGraphPathPropertyId`).

---

## Project scripts

Lua (LuaBridge3, `thirdparty/LuaBridge3`) is project-level, not per-component. `ScriptObjectSystem` (`src/Editor/ECS/Script/ScriptObjectSystem.h`) is a `MikanTypedObjectSystem` registered last in `ProjectManager::startup`, so every other system's objects already exist when scripts run. Each `ScriptComponent` (definition `ScriptDefinition`, property `script_path`) names one script file through a `ScriptAssetReference` (`*.lua`). The system owns the project's single `ProjectScriptContext`, one `lua_State` shared by every script.

On `postInit`, and on the next `update` after a script is added, removed, or has its path changed (`requestReload`), the system calls `reloadAllScripts()`: dispose the current state, create a fresh one, then run every script with a non-empty path in pool order (`runScriptFile`), and bind the result to `ScriptRequestHandler`. With no scripts in the pool there is no Lua state at all. If any file fails to run, the whole state is disposed and an error names the failing script's id and path. A single bad file takes down every script, not just its own.

The outliner's project root has a top-level Scripts folder. Its add button calls `ScriptObjectSystem::addNewScript()`, which creates `<project>/scripts/script_<timestamp>.lua` and registers it. Selecting a script row shows its path (click to pick a different `.lua` file), Edit and Reload buttons, and one button per trigger that file registered. Delete uses the outliner's Delete Component button. Reload re-runs every script in the pool, since they share one state.

`CommonScriptContext` (`Scripting/CommonScriptContext.h`) owns the `lua_State`: standard libs opened, a panic handler and detailed error reporting (`checkLuaResult` logs Lua tracebacks and disposes the state on error), and a built-in coroutine scheduler. The scheduler injects globals `start_coroutine`, `wait_frames`, `wait_next_frame`, `wait_seconds`, `get_frame_delta_seconds`. `update_scheduler()` is invoked once per frame from `updateScript()`. `runScriptFile(path, scriptId)` runs one file's chunk into the shared state. While the chunk runs, `m_loadingScriptId` holds the running script's id so any registration it makes is attributed to it.

Scripts declare their entry points through the `ScriptContext` namespace. Each registration is attributed to whichever script file was executing when it was called:

- `ScriptContext.registerTrigger(functionName)`: exposes a named global function as a trigger.
- `ScriptContext.registerMessageHandler(functionName)`: handler receives a string message, returns true if handled.
- `ScriptContext.registerHttpTrigger(routeName, functionName)`: binds a trigger to the HTTP route `/trigger/<routeName>`.
- `ScriptContext.broadcastMessage(message)`: emits a message to connected clients.

`ScriptComponent::getTriggerNames()` and `invokeTrigger()` filter to the triggers registered by that component's own file (`CommonScriptContext::getTriggerNamesForScript(scriptId, ...)`).

What is scriptable: `ProjectScriptContext::bindContextFunctions()` binds LuaBridge classes for the object systems (`CameraObjectSystem`, `SceneObjectSystem`, `AnchorObjectSystem`, `CompositorObjectSystem`, `DMXObjectSystem`, the stencil and shape systems) and components (`MikanComponent` and subclasses: transform, scene, stage, camera, compositor, stencils, shapes, anchor, marker, DMX fixture, RGB lights). It sets these globals, one per scriptable object system:

- `CameraSystem`
- `SceneSystem`
- `AnchorSystem`
- `CompositorSystem`
- `DMXSystem`
- `ModelStencilSystem`
- `BoxStencilSystem`
- `QuadStencilSystem`
- `ModelShapeSystem`
- `BoxShapeSystem`
- `QuadShapeSystem`

There is no `ownerComponent` global: a script is not bound to a single component, so it reaches objects through the system globals above. A component handle still exposes `getCameraSystem()`, `getSceneSystem()`, `getDMXSystem()`, `getAnchorSystem()`, `getCompositorSystem()` methods for scripts that already hold a component reference. Math helpers `LuaVec3f`/`LuaQuatf` come from `Scripting/LuaMath.h`. Enum constants (e.g. `eStencilCullMode`) are registered as globals.

---

## Script RPC surface

`ScriptRequestHandler` (`src/Editor/Server/ScriptRequestHandler.h`) is the server-side bridge (see [wire-protocol.md](./wire-protocol.md)). It holds one bound `ProjectScriptContext` at a time (`bindScriptContext`/`unbindScriptContext`/`getScriptContext`):

- `InvokeScriptTrigger { trigger_name }` request: triggers are project-wide, so the request carries only the trigger name. Result codes:
	- `MalformedParameters`: the trigger name is not registered.
	- `RequestFailed`: no project script state is loaded, or the trigger itself failed.
	- `Success`: otherwise.

- `SendScriptMessage` request: offers the message to the bound context's handlers in registration order until one returns true. An unhandled message is still `Success`.

- `MikanScriptMessagePostedEvent`: published to clients whenever a script calls `broadcastMessage`.

- HTTP triggers: `registerHttpTriggerRoute(routeName, triggerName)` installs routes under `/trigger/<name>` on the HTTP message server (Stream Deck style integrations) for every `ScriptContext.registerHttpTrigger` binding the bound context declared. Responses are JSON result codes mapped from `MikanAPIResult`.

---

## Lua debugging

`LuaDebugServer` (`Scripting/LuaDebugServer.h`) is a singleton LRDB debug server on TCP port 21110 (the vscode-lrdb extension default). `MainWindow` starts it listening before the first project loads, and calls `poll()` every frame. The project's `ProjectScriptContext` attaches itself when its Lua state is created (`createScriptState`) and detaches when the state is disposed (`disposeScriptState`), so breakpoints in any project script work without a manual attach step. Scripts can call `lrdb_break()` for a programmatic breakpoint. Chunk names are made workspace-relative so VSCode gutter breakpoints match. See [debugging.md](./debugging.md).
