function Set_desk_camera()
    local scene = ownerComponent:getSceneSystem():getSceneByName("MyScene")
    local compositor = ownerComponent:getCompositorSystem():getCompositorByName("DeskCompositor")

    if scene ~= nil and compositor ~= nil then
        scene.displayCompositorId = compositor.compositorId
    end
end

function Set_slider_camera()
    local scene = ownerComponent:getSceneSystem():getSceneByName("MyScene")
    local compositor = ownerComponent:getCompositorSystem():getCompositorByName("SliderCompositor")

    if scene ~= nil and compositor ~= nil then
        scene.displayCompositorId = compositor.compositorId
    end
end

ScriptContext.registerTrigger("Set_desk_camera")
ScriptContext.registerTrigger("Set_slider_camera")

ScriptContext.registerHttpTrigger("compositor/desk_camera", "Set_desk_camera")
ScriptContext.registerHttpTrigger("compositor/slider_camera", "Set_slider_camera")
