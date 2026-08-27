# Scripting

MikanXR has two programmable layers: visual node graphs (`src/Editor/NodeEditors/`) that drive rendering pipelines, and Lua component scripts (`src/Editor/Scripting/`) that drive scene logic. The compositor graph's rendering semantics are covered in [compositor.md](./compositor.md), the ECS objects scripts manipulate in [objects.md](./objects.md), and the client-facing script RPC in [wire-protocol.md](./wire-protocol.md).

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

## Lua component scripts

Lua (LuaBridge3, `thirdparty/LuaBridge3`) attaches at the component level. Any `MikanComponent` definition can carry a script path (a `ScriptAssetReference`, `*.lua`). `MikanComponent::initScriptContext()` creates a `ComponentScriptContext`, loads the script, registers the context with `ScriptRequestHandler`, and subscribes to the owner object system's `onUpdate` so `CommonScriptContext::updateScript()` runs every tick.

`CommonScriptContext` (`Scripting/CommonScriptContext.h`) owns one `lua_State` per script: standard libs opened, a panic handler and detailed error reporting (`checkLuaResult` logs Lua tracebacks and disposes the state on error), and a built-in coroutine scheduler. The scheduler injects globals `start_coroutine`, `wait_frames`, `wait_next_frame`, `wait_seconds`, `get_frame_delta_seconds`; `update_scheduler()` is invoked once per frame from `updateScript()`.

Scripts declare their entry points through the `ScriptContext` namespace:

- `ScriptContext.registerTrigger(functionName)`: exposes a named global function as a trigger.
- `ScriptContext.registerMessageHandler(functionName)`: handler receives a string message, returns true if handled.
- `ScriptContext.registerHttpTrigger(routeName, functionName)`: binds a trigger to the HTTP route `/trigger/<routeName>`.
- `ScriptContext.broadcastMessage(message)`: emits a message to connected clients.

What is scriptable: `ComponentScriptContext::bindContextFunctions()` binds LuaBridge classes for the object systems (`CameraObjectSystem`, `SceneObjectSystem`, `AnchorObjectSystem`, `CompositorObjectSystem`, `DMXObjectSystem`, the stencil and shape systems) and components (`MikanComponent` and subclasses: transform, scene, stage, camera, compositor, stencils, shapes, anchor, marker, DMX fixture, RGB lights). It sets the global `ownerComponent` to the script's owning component and exposes the stencil/shape system singletons as globals. Math helpers `LuaVec3f`/`LuaQuatf` come from `Scripting/LuaMath.h`. Enum constants (e.g. `eStencilCullMode`) are registered as globals.

Reload is live: the `reload_script` component function (also in the UI) unbinds the context from the server, reloads the file, and rebinds, refreshing HTTP trigger routes.

---

## Script RPC surface

`ScriptRequestHandler` (`src/Editor/Server/ScriptRequestHandler.h`) is the server-side bridge (see [wire-protocol.md](./wire-protocol.md)):

- `InvokeComponentScriptTrigger` request: resolves owner system name + component id, invokes the named trigger on that component's script context.
- `SendScriptMessage` request: offers the message to each bound script context until one handler returns true.
- `MikanScriptMessagePostedEvent`: published to clients whenever a script calls `broadcastMessage`.
- HTTP triggers: routes registered under `/trigger/<name>` on the HTTP message server (Stream Deck style integrations); responses are JSON result codes mapped from `MikanAPIResult`.

---

## Lua debugging

`LuaDebugServer` (`Scripting/LuaDebugServer.h`) is a singleton LRDB debug server on TCP port 21110 (the vscode-lrdb extension default). `MainWindow` starts it at startup and calls `poll()` every frame. An RAII `LuaDebugContextGuard` attaches the debugger to whichever script context is currently executing, so breakpoints follow the active script without manual attach. Scripts can call `lrdb_break()` for a programmatic breakpoint. Chunk names are made workspace-relative so VSCode gutter breakpoints match. See [debugging.md](./debugging.md).
