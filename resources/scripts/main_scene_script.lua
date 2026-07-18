function Set_desk_camera()
    
end

function Set_slider_camera()
    
end

ScriptContext.registerHttpTrigger("/compositor/desk_camera", "Set_desk_camera")
ScriptContext.registerHttpTrigger("/compositor/slider_camera", "Set_slider_camera")