local easing = require("resources/scripts/easing")

local function torii_scale_bounce_coroutine()
  local model_stencil_list= ModelStencilList.new()
  local torii_stencil= ModelStencil.new(model_stencil_list:getStencilId(0))
  local final_scale= torii_stencil:getModelScale()
  local elapsed_time = 0
  local duration = 2
  
  while (elapsed_time < duration)
  do
    local start= 0
    local change= 1 - start
    local s = easing.outBounce(elapsed_time, start, change, duration)
    local current_scale = Vec3fMath.scaleUniform(final_scale, s)
    torii_stencil:setModelScale(current_scale)
    wait_next_frame()
    elapsed_time= elapsed_time + get_frame_delta_seconds()
  end
end

local function quad_scale_bounce_coroutine()
  local quad_stencil_list= QuadStencilList.new()
  local quad_stencil= QuadStencil.new(quad_stencil_list:getStencilId(0))
  local final_width= quad_stencil:getQuadWidth()
  local final_height= quad_stencil:getQuadHeight()
  local elapsed_time = 0
  local duration = 2
  
  while (elapsed_time < duration)
  do
    local start= 0
    local change= 1 - start
    local s = easing.outBounce(elapsed_time, start, change, duration)
    quad_stencil:setQuadWidth(s*final_width)
    quad_stencil:setQuadHeight(s*final_height)
    wait_next_frame()
    elapsed_time= elapsed_time + get_frame_delta_seconds()
  end
end

local function stencil_bounce_coroutine()
  start_coroutine(torii_scale_bounce_coroutine)
  wait_seconds(0.5)
  start_coroutine(quad_scale_bounce_coroutine)
end

function play_stencil_scale_bounce()
  start_coroutine(stencil_bounce_coroutine)
end

ScriptContext.registerTrigger("play_stencil_scale_bounce")