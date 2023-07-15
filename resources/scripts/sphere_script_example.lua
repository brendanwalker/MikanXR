local easing = require("resources/scripts/easing")

local function sphere_scale_bounce_coroutine()
  local model_stencil_list= ModelStencilList()
  local sphere_stencil= ModelStencil(model_stencil_list:getStencilId(3))
  local final_scale= Vec3f(10.0, 10.0, 10.0)
  local elapsed_time = 0
  local duration = 1.5
  
  while (elapsed_time < duration)
  do
    local start= 0
    local change= 1 - start
    local s = easing.outBounce(elapsed_time, start, change, duration)
    local current_scale = final_scale:scaleUniform(s)
    sphere_stencil:setModelScale(current_scale)
    wait_next_frame()
    elapsed_time= elapsed_time + get_frame_delta_seconds()
  end
end

function play_stencil_scale_bounce()
  start_coroutine(sphere_scale_bounce_coroutine)
  ScriptContext.broadcastMessage("startCameraZoom")
end

function reset_stencil_scale()
    local model_stencil_list= ModelStencilList()
    local sphere_stencil= ModelStencil(model_stencil_list:getStencilId(3))
    local reset_scale= Vec3f(0.0, 0.0, 0.0)
    sphere_stencil:setModelScale(reset_scale)
    ScriptContext.broadcastMessage("resetCameraZoom")
end

ScriptContext.registerTrigger("play_stencil_scale_bounce")
ScriptContext.registerTrigger("reset_stencil_scale")