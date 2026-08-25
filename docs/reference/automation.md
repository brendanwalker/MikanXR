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

Growth convention: when a feature adds automation commands, register the namespace in `AutomationServer::registerCoreNamespaces` (or from the owning subsystem) and add its section here.
