-- A stream event that takes the scene over for a few seconds and gives it back.
--
-- An external tool (Streamer.bot, a Stream Deck, anything that can fetch a URL)
-- calls the route with the event's details in the query string:
--
--   http://127.0.0.1:8090/trigger/gift_sub?user=bob&tier=3
--
-- The trigger function receives those pairs as a table. Values are always
-- strings, so anything numeric needs tonumber(). The table is always present
-- and is empty when the caller sent no query string, as the panel button does.

ScriptContext.registerVariable("effect_scene_name", "SubCelebration")
ScriptContext.registerVariable("effect_seconds", 6.0)
ScriptContext.registerVariable("effect_sequence", "SEQ_Rainbow")

-- The scene to return to, and the guard that keeps a second event arriving
-- mid-effect from caching the effect scene as the one to restore
local cached_scene_id = nil

function on_gift_sub(args)
	if cached_scene_id ~= nil then
		-- Already running. Anything but returning here restores the wrong scene.
		return
	end

	local current_scene = SceneSystem:getCurrentScene()
	local effect_scene = SceneSystem:getSceneByName(effect_scene_name)
	if current_scene == nil or effect_scene == nil then
		return
	end

	cached_scene_id = current_scene.componentId

	local user = args.user or "someone"
	local tier = tonumber(args.tier) or 1

	SceneSystem:setCurrentScene(effect_scene)

    local sequence = DMXSequenceSystem:getSequenceByName(effect_sequence)
    if sequence ~= nil then
        sequence:play()
    end

	-- Reaches connected clients (the Unreal plugin, a browser overlay) as a
	-- MikanScriptMessagePostedEvent, so they can play their own effects
	ScriptContext.broadcastMessage(string.format("gift_sub:%s:%d", user, tier))

	start_coroutine(function()
		wait_seconds(effect_seconds)

		SceneSystem:setCurrentSceneById(cached_scene_id)
		cached_scene_id = nil
	end)
end

-- Both registrations are needed: registerTrigger is what makes the function
-- callable at all (and gives the script panel its button), registerHttpTrigger
-- only maps a route onto it.
ScriptContext.registerTrigger("on_gift_sub")
ScriptContext.registerHttpTrigger("gift_sub", "on_gift_sub")
