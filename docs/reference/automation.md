# Automation server

How to drive and inspect a running `Mikan.exe` over the automation server: a loopback TCP text channel for test scripts and AI tooling. It is deliberately separate from the websocket client API, which serves shipped client applications ([wire-protocol.md](./wire-protocol.md)), and it complements the headless `MikanCmd.exe` commands ([commands.md](./commands.md)), which cover code paths that need no running editor. The server lives in `src/Editor/Server/AutomationServer` over the line socket in `src/Editor/Interprocess/AutomationSocket`, pumped once per frame on the main thread from `MainWindow::update`, so commands run synchronously against editor state.

---

## Enabling and port

The server always starts with `Mikan.exe` and listens on 127.0.0.1 only. The default port is 21120 (clear of the websocket 8080, HTTP 8090, and Lua debugger 21110 ports).

- The port persists in `AppSettingsConfig` under `automationServerPort`.
- `-automationPort=<n>` on the command line overrides the setting.
- `-noAutomationServer` skips starting the listener.
- A failed bind logs an error and the editor runs on without the channel.

## Protocol

One client at a time. Commands are newline-terminated text lines (an optional trailing `\r` is stripped). Tokens split on spaces and tabs; a double-quoted span groups spaces into one token, with `\"` and `\\` escapes inside quotes.

Every command answers with one framed reply: a line holding the reply's content line count, then exactly that many content lines. A single value is `1` then the value. A success with nothing to say is just `0`. An error is `1` then one line `<namespace> <verb>: <message>`. An unknown namespace replies naming the valid namespaces. Nothing falls through silently, so a client awaits each reply and stays in sync.

Stage transitions land on the frame after the command that requested them, so a drive that pushes a stage waits a frame (or polls `app info`) before acting on the new stage.

## Commands

`help` lists every registered command.

### App control (app)

- `app info` replies the current stage name, the parent stage name (`none` at the root), and the loaded project path (`none` when no project is loaded)
- `app push <stageName>` / `app pop` push a registered app stage by name, or pop back to the parent stage
- `app open <projectPath>` / `app new <projectPath>` / `app resume` open, create, or resume a project (routed to the main menu stage's remote control commands, so they answer an error outside the main menu)
- `app quit` requests a clean shutdown, the same path as the window close button

### Stage commands (stage)

`stage <command> [parameters...]` passes a command to the current app stage's `IRemoteControllable::handleRemoteControlCommand`, the same channel the websocket `MikanRemoteControlCommand` request uses. The stage's result strings become the reply lines. The calibration stages answer commands such as `get_state` and `begin`; a stage that does not recognize the command answers an error naming the stage.

`AlignCameraByOriginMarker` answers four, which is enough to drive a whole alignment headlessly:

- `get_state` replies the menu state (`verifySetup`, `capturing`, `testCalibration`, ...)
- `get_marker_visible` replies whether the origin marker is currently detected
- `begin` starts sampling, failing unless the stage is in `verifySetup` with the marker visible
- `restart` discards the samples and returns to `verifySetup`

Reach the stage with `function invoke CameraObjectSystem <cameraId> align_camera`, which is what the editor's own Align Camera button calls.

### Scene introspection (system, component, property, function)

These reach every object system and component through the property and function databases ([objects.md](./objects.md)).

- `system list` replies one object system class name per line
- `component list <system> [componentClass]` replies `<componentId> <componentClass>` per live component
- `component create <system> <componentClass>` creates an object of the system's primary component type with a default definition, replying the new component id
- `component destroy <system> <componentId>` destroys the object owning the component
- `property list [system] [componentClass]` replies `<system> <componentClass> <name> <type> <ro|rw>` per property
- `property get <system> <componentId> <name>` replies the value as text
- `property set <system> <componentId> <name> <value...>` writes a property, coercing the value tokens to the descriptor's type
- `function list [system] [componentClass]` replies `<system> <componentClass> <functionName> <displayName>` per function
- `function invoke <system> <componentId> <name>` invokes a function (functions take no arguments; the pattern is set properties, then invoke)

A component id of `-1` targets the system itself in `property get`/`set` and `function invoke`.

A `property set` mutates the component's definition, and the project autosave persists it to the project file a few seconds later. A drive that mutates project state restores the original value and waits out the autosave cooldown before quitting, or its last write may not be the one on disk.

Value syntax for `property get`/`set`:

- booleans are `true`/`false`
- vectors take one numeric token per component (`property set ... relative_position 0 1.5 0`)
- quaternions order their components w x y z
- matrices take 16 floats in `MikanMatrix4f` field order (x0 x1 x2 x3 y0 ... w3)
- strings with spaces are quoted (`"My Anchor"`); a bare string may span tokens, which rejoin with single spaces
- array, map, and object typed properties read back as formatted text but refuse a set

### Screenshots (screenshot)

- `screenshot compositor [componentId] [path]` writes the composited frame to a PNG and replies the absolute path. The component id may be omitted when the project has exactly one compositor. Without a path it writes `mikan_compositor.png` in the working directory, overwritten each capture.
- `screenshot window [path]` writes the main window's back buffer (the editor UI included) at the end of the current frame's render, replying the absolute path once the capture lands. Default `mikan_window.png`.

### Lua scripting (script)

- `script list` replies `<system> <componentId> <scriptFile> [triggers]` per bound script context
- `script eval <system> <componentId> <lua-code>` runs a Lua statement in the component's script context and replies what it returns (the code is the raw untokenized rest of the line, so Lua quotes pass through verbatim: `script eval SceneObjectSystem 1052 return 1+1`)
- `script trigger <system> <componentId> <triggerName>` invokes a script trigger, the same call the HTTP trigger routes make

### Log access (log)

`log tail <lineCount> [minLevel]` replies the most recent log lines at or above the level, oldest first, from an in-process ring of the last 2000 lines. Levels are the logger's names:

- trace
- debug
- info
- warning
- error
- fatal

### Transaction history (history)

The editor transaction system's automation face ([transactions.md](./transactions.md)). Every persistent edit records as an undoable transaction, and a JSONL session log persists beside the project for post-session diagnosis.

- `history list [n]` replies the last n transactions as `<seq> <applied|undone> <description>`
- `history info` replies depth, cursor, can_undo/can_redo, the open gesture, and the session log path
- `history undo [n]` / `history redo [n]` step the stack, replying the resulting cursor
- `history clear` drops the in-memory history (the log keeps its record)

A drive that mutates state can restore it exactly with `history undo` instead of hand-reverting property values, and a session that went wrong reads back from the log file even after a crash.

### Node graph editing (nodegraph)

Drives the node editor window and its snapshot undo history ([transactions.md](./transactions.md)). One node editor window is targeted at a time; the commands answer an error when none is open.

- `nodegraph open [compositorComponentId]` opens the compositor graph editor window (the id may be omitted when the project has exactly one compositor)
- `nodegraph close` asks the app to tear the window down at the end of the frame
- `nodegraph info` replies the graph class and path, node/pin/link/property counts, `can_undo`/`can_redo`, the history depth and cursor, the session log path ([transactions.md](./transactions.md)), and for the compositor editor a `running` line
- `nodegraph list nodes|pins|links|properties` replies one object per line: nodes as `<id> <class> <editorTitle>`, pins adding owner node id, direction, and name, links as `<id> <startPinId> <endPinId>`
- `nodegraph createnode <nodeClassName> [x y]` creates a node at the grid position, replying the new node id
- `nodegraph deletenode <nodeId>` deletes a node with its pins and links
- `nodegraph createlink <startPinId> <endPinId>` connects two compatible pins, replying the new link id
- `nodegraph deletelink <linkId>` deletes a link
- `nodegraph undo [n]` / `nodegraph redo [n]` step the window's snapshot history, replying the resulting cursor
- `nodegraph run on|off` pauses or resumes compositor evaluation of the editor graph (the Compositor menu's Run item), replying the resulting state
- `nodegraph renamevar <propertyId> <name...>` renames a graph variable (the name is the rest of the line, so spaces survive)
- `nodegraph reordervar <movedPropertyId> <targetPropertyId>` moves a variable to the target's slot in the list, the headless equivalent of dragging one variable row onto another

`nodegraph list properties` replies in variable-list order, so a reorder is observable there.

### ARKit debug channel (arkit)

Relays debug traffic to and from the MikanARStreamer iPhone app ([videosources.md](./videosources.md)). The phone pushes diagnostics, which are re-emitted through the editor's own logger and so read back through `log tail` interleaved with editor lines on one timeline. Commands travel the other way.

- `arkit status` replies whether the listener is enabled, its port, whether a phone is connected, and that phone's address, device name, and protocol version
- `arkit send <text...>` sends one command line to the phone and replies with the phone's own reply

The channel is off by default. Unlike this server it binds every interface, because its peer is a phone on the LAN, so it is opt-in: `arkitDebugChannelEnabled` in `AppSettingsConfig` (port `arkitDebugChannelPort`, default 21121), or `-arkitDebugChannel` on the command line with `-arkitDebugPort=<n>` to override the port. The `arkit` namespace registers either way, so `arkit status` still answers when the listener is off.

`arkit send` takes the raw rest of the line, so quoting reaches the phone verbatim. Its reply is parked until the phone answers rather than being answered immediately, and a phone that goes quiet fails the command after five seconds instead of leaving the client waiting. Only one command is in flight at a time. The command text is opaque to the editor: the phone owns its own vocabulary, so it can grow without an editor rebuild.

Only one peer is accepted at a time, so a peer that connects and then never sends its hello is dropped after ten seconds rather than holding the slot. Without that, one wedged client locks the channel until the editor restarts.

The commands the MikanARStreamer app answers today:

- `ping` replies `pong`
- `stats` replies the capture, encode, drop, and send counters as `name value` lines
- `verbose on|off` gates the per-frame encode-latency relay, which is off by default because one line per frame fills the 2000-line log ring in about a minute and evicts the editor's own diagnostics
- `screenshot [name]` captures the app's own UI to a PNG in its container and replies with the path, pixel size, and byte count

The screenshot command exists because nothing else can see that screen. `devicectl` can copy files off a device but cannot capture one, and `simctl io screenshot` is simulator only, so the app takes the picture itself and leaves it where a copy can reach:

```
python tools/automate.py "arkit send screenshot land"
xcrun devicectl device copy from --device <deviceId> \
  --domain-type appDataContainer --domain-identifier com.mikan.ARStreamer \
  --source Documents/land.png --destination ./land.png
```

The image travels as a file rather than as base64 through the channel, which keeps a line-oriented text protocol from carrying megabytes. Note that `devicectl device orientation set` is not available on every device (an iPhone 12 Pro reports the capability as unsupported), so a layout has to be checked in whichever orientation the phone is physically in.

A real phone session can be driven without touching the device. The app's settings live in `UserDefaults`, so launch arguments override them for that launch only, and `-autostart 1` starts streaming without a tap:

```
xcrun devicectl device process launch --device <deviceId> com.mikan.ARStreamer \
  -- -settings.host <editorHost> -settings.basePort 27015 -autostart 1
```

The phone must be unlocked for install, and ARKit still needs textured surroundings for the pose to mean anything.

`tools/arkit_debug_stub.py` stands in for the phone, which is how the channel is tested without a device on the bench. It answers `ping`, `stats`, and `empty`, and deliberately ignores `silent` so the timeout path can be exercised.

```
build/src/Editor/Release/Mikan.exe -arkitDebugChannel
python tools/arkit_debug_stub.py
python tools/automate.py "arkit status" "arkit send ping" "log tail 20 info"
```

Mutations and undo/redo run inside the node editor window's next update (its GL and gui contexts are only current there), so those replies land a frame late, and a mutation's undo snapshot commits on the window's next quiescent frame. A drive polls `nodegraph info` (or rides automate.py's inter-command delay) before asserting `can_undo`.

## Client helper

`tools/automate.py` is the checked-in client: it connects (retrying a refused port until `--wait` seconds, default 20), sends each argument as one command, prints each framed reply, and exits nonzero on a connection failure or timeout. `--port` overrides the default 21120, `--delay` sets the pause between commands (default 0.1 s, enough for a stage transition to land), and `--timeout` bounds each reply and `until` poll (default 60 s).

```
python tools/automate.py "app info"
python tools/automate.py "app resume" sleep:1 "screenshot compositor"
python tools/automate.py "property get CompositorObjectSystem 1051 spout_output_name"
```

Three arguments run client-side instead of being sent:

- `ready` waits until the server answers `app info` (the connect retry already covers a fresh launch)
- `sleep:<seconds>` pauses that long instead of the default delay
- `until:<command>:<expected>` polls the command until a reply line starts with the expected text, so a drive waits for a state edge instead of sleeping out a worst-case budget

## The drive and verify loop

The standard way to verify an editor feature objectively: launch `Mikan.exe`, drive it with commands, then read state back rather than assuming behavior.

```
./build/src/Editor/Release/Mikan.exe &
python tools/automate.py "app resume" "until:app info:stage Compositor" "screenshot compositor"
python tools/automate.py "property get <system> <id> <name>" "log tail 20 error" "app quit"
```

`app push <stageName>` enters a stage but gives it no target, so a stage that normally acts on a selected object comes up empty. Reach those through the owning component instead: `function invoke ARKitVideoObjectSystem <componentId> show_video_source_settings` is what the editor's own button calls, and it both enters `VideoSourceSettings` and binds the source to it. A bare `app push VideoSourceSettings` reports the expected stage from `app info` while showing no video at all, which reads like a broken device rather than a missing selection. Video sources open on demand from that path too, so nothing binds the receive socket until it runs.

A handler that cannot answer straight away (the ARKit channel's `arkit send` is the one case today) calls `AutomationServer::deferReply`, then answers later through `sendDeferredReply`. Nothing bounds that wait to the current frame, so a handler that defers owns arming its own timeout.

Growth convention: when a feature adds automation commands, register the namespace in `AutomationServer::registerCoreNamespaces` (or from the owning subsystem) and add its section here.
