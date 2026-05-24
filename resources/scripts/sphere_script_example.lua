local easing = require("resources/scripts/easing")

local function sphere_scale_bounce_coroutine()
  local sphere_stencil = ModelStencilSystem:getModelStencilByName("Sphere")
  if sphere_stencil == nil then
    print("sphere_script_example: could not find ModelStencil named 'Sphere'")
    return
  end

  local final_scale = Vec3f(10.0, 10.0, 10.0)
  local elapsed_time = 0
  local duration = 1.5

  while (elapsed_time < duration)
  do
    local start = 0
    local change = 1 - start
    local s = easing.outBounce(elapsed_time, start, change, duration)
    local current_scale = final_scale:scaleUniform(s)
    sphere_stencil.relativeScale = current_scale
    wait_next_frame()
    elapsed_time = elapsed_time + get_frame_delta_seconds()
  end
end

function Play_stencil_scale_bounce()
  start_coroutine(sphere_scale_bounce_coroutine)
  ScriptContext.broadcastMessage("startCameraZoom")
end

function Reset_stencil_scale()
  local sphere_stencil = ModelStencilSystem:getModelStencilByName("Sphere")
  if sphere_stencil == nil then
    print("sphere_script_example: could not find ModelStencil named 'Sphere'")
    return
  end
  sphere_stencil.relativeScale = Vec3f(0.0, 0.0, 0.0)
  ScriptContext.broadcastMessage("resetCameraZoom")
end

ScriptContext.registerTrigger("Play_stencil_scale_bounce")
ScriptContext.registerTrigger("Reset_stencil_scale")
