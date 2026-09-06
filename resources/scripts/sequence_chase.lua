-- A chase: one lit fixture runs along the members of a DMXFixtureGroup.
-- Register it here, then point a DMXSequenceComponent at the "chase" handler
-- and press Play. The handler writes into the sequence's frame buffer; the
-- sequence system pushes that buffer to the fixtures after every update.

ScriptContext.registerVariable("chase_speed", 2.0) -- fixtures per second

local chase = {}

function chase.start(sequence)
	sequence:fillGroup(0, 0, 0)
end

function chase.update(sequence, timeSinceStart, deltaSeconds)
	local group = sequence:getGroup()
	if group == nil then
		return
	end

	local count = group:getFixtureCount()
	if count == 0 then
		return
	end

	local head = math.floor(timeSinceStart * chase_speed) % count
	sequence:fillGroup(0, 0, 0)
	sequence:setFixtureColor(group:getFixtureAtIndex(head).componentId, 255, 40, 0)
end

-- Optional: Stop leaves the last frame unless the handler writes another
function chase.stop(sequence)
	sequence:fillGroup(0, 0, 0)
end

ScriptContext.registerSequence("chase", chase)
