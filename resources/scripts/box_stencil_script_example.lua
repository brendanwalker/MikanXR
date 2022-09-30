local easing = require("resources/scripts/easing")

local function box_scale_bounce_coroutine()
  local box_stencil_list= BoxStencilList()
  local box_stencil= BoxStencil(box_stencil_list:getStencilId(0))
  local final_size= Vec3f(box_stencil:getBoxXSize(), box_stencil:getBoxYSize(), box_stencil:getBoxZSize())
  local elapsed_time = 0
  local duration = 2
  
  while (elapsed_time < duration)
  do
    local start= 0
    local change= 1 - start
    local s = easing.outBounce(elapsed_time, start, change, duration)
    local current_size = final_size:scaleUniform(s)
    box_stencil:setBoxXSize(current_size.x)
    box_stencil:setBoxYSize(current_size.y)
    box_stencil:setBoxZSize(current_size.z)
    wait_next_frame()
    elapsed_time= elapsed_time + get_frame_delta_seconds()
  end
end

function play_stencil_scale_bounce()
  start_coroutine(box_scale_bounce_coroutine)
end

ScriptContext.registerTrigger("play_stencil_scale_bounce")