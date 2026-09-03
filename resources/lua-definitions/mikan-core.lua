---@meta
-- Mikan scripting API — core types, globals, and coroutine scheduler.
-- This file is a LuaLS definition file; it contains no executable code.
-- Keep in sync with: src/Editor/Scripting/CommonScriptContext.cpp (bindCommonScriptFunctions,
-- addLuaCoroutineScheduler) and src/Editor/Scripting/LuaMath.cpp (LuaVec3f::bindFunctions,
-- LuaQuatf::bindFunctions).

------------------------------------------------------------------------
-- Coroutine scheduler globals (injected by addLuaCoroutineScheduler)
------------------------------------------------------------------------

--- Wait for `frame_duration` frames before resuming the current coroutine.
---@param frame_duration number
function wait_frames(frame_duration) end

--- Wait until the next frame before resuming the current coroutine.
function wait_next_frame() end

--- Wait for `seconds_duration` seconds before resuming the current coroutine.
---@param seconds_duration number
function wait_seconds(seconds_duration) end

--- Returns the duration of one frame in seconds (1/fps).
---@return number
function get_frame_delta_seconds() end

--- Start a new coroutine. The task function runs asynchronously across frames.
---@param task function
function start_coroutine(task) end

------------------------------------------------------------------------
-- Debugger
------------------------------------------------------------------------

--- Trigger a pause in the LRDB Lua debugger on the next line event.
--- Use this as a programmatic breakpoint when gutter breakpoints cannot
--- be set through the VS Code extension.
function lrdb_break() end

------------------------------------------------------------------------
-- Vec3f — 3-component float vector
------------------------------------------------------------------------

---@class Vec3f
---@field x number X component
---@field y number Y component
---@field z number Z component
---@overload fun(x: number, y: number, z: number): Vec3f
Vec3f = {}

--- Add two vectors.
---@param v Vec3f
---@return Vec3f
function Vec3f:__add(v) end

--- Subtract two vectors.
---@param v Vec3f
---@return Vec3f
function Vec3f:__sub(v) end

--- Scale uniformly by a scalar.
---@param s number
---@return Vec3f
function Vec3f:scaleUniform(s) end

--- Scale non-uniformly by a per-component vector.
---@param s Vec3f
---@return Vec3f
function Vec3f:scaleNonUniform(s) end

--- Return the Euclidean length of the vector.
---@return number
function Vec3f:length() end

--- Return a unit-length copy of the vector.
---@return Vec3f
function Vec3f:normalize() end

--- Compute the dot product of two vectors.
---@param a Vec3f
---@param b Vec3f
---@return number
function Vec3f.dot(a, b) end

--- Compute the cross product of two vectors.
---@param a Vec3f
---@param b Vec3f
---@return Vec3f
function Vec3f.cross(a, b) end

------------------------------------------------------------------------
-- Quatf — unit quaternion rotation
------------------------------------------------------------------------

---@class Quatf
---@field w number W component
---@field x number X component
---@field y number Y component
---@field z number Z component
---@overload fun(w: number, x: number, y: number, z: number): Quatf
Quatf = {}

--- Compose (multiply) two quaternions.
---@param q Quatf
---@return Quatf
function Quatf:__mul(q) end

--- Rotate a vector by this quaternion.
---@param v Vec3f
---@return Vec3f
function Quatf:rotateVec3f(v) end

--- Return the multiplicative inverse of this quaternion.
---@return Quatf
function Quatf:inverse() end

--- Return the norm (length) of this quaternion.
---@return number
function Quatf:length() end

--- Return a unit-length copy of this quaternion.
---@return Quatf
function Quatf:normalize() end

------------------------------------------------------------------------
-- ScriptContext — script lifecycle and messaging helpers
------------------------------------------------------------------------

---@class ScriptContext
--- All project scripts share one Lua state. `registerTrigger`, `registerMessageHandler`,
--- and `registerHttpTrigger` attribute the registration to whichever script file is
--- currently being loaded.
ScriptContext = {}

--- Register a global Lua function as a trigger.
--- Triggers are called by Mikan in response to UI Button Events.
---@param functionName string Name of the global function to register.
function ScriptContext.registerTrigger(functionName) end

--- Register a global Lua function as a message handler.
--- The handler receives a single string argument and should return true if handled.
---@param functionName string Name of the global function to register.
function ScriptContext.registerMessageHandler(functionName) end

--- Register a global Lua function as a http trigger.
--- HTTP triggers are called by Mikan in response to HTTP requests.
---@param routeName string HTTP route to register the trigger for.
---@param functionName string Name of the global function to register.
function ScriptContext.registerHttpTrigger(routeName, functionName) end


--- Broadcast a string message to all registered message handlers in all contexts.
---@param message string
function ScriptContext.broadcastMessage(message) end

------------------------------------------------------------------------
-- ScriptContext.CullMode enum
------------------------------------------------------------------------

--- Stencil cull-mode values. Use with StencilComponent.cullMode.
---@alias ScriptContext.CullMode
---| 0 # none — no culling
---| 1 # zAxis — cull along Z axis
---| 2 # yAxis — cull along Y axis
---| 3 # xAxis — cull along X axis

ScriptContext.CullMode = { none = 0, zAxis = 1, yAxis = 2, xAxis = 3 }
