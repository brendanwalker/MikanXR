#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "DMXFixtureComponent.h"
#include "LightSystemFwd.h"
#include "MikanComponent.h"
#include "MikanLightTypes.h"
#include "MikanTypeFwd.h"
#include "ScriptingFwd.h"

#include <cstdint>
#include <string>
#include <vector>

enum class eDMXSequenceState : int
{
	Stopped= 0,
	Playing= 1,
	Paused= 2,
};

// -- DMXSequenceDefinition -----
// A Lua-driven animation of one fixture group: the named handler, registered
// by a project script through ScriptContext.registerSequence, is called every
// frame while the sequence plays and writes into the sequence's frame buffer.
class DMXSequenceDefinition : public MikanComponentDefinition
{
public:
	DMXSequenceDefinition();
	DMXSequenceDefinition(MikanDMXSequenceID sequenceId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;
	virtual bool readFromInitParams(MikanObjectSystem* ownerObjectSystem,
									const Serialization::PolymorphicObjectPtr& initParams) override;

	inline MikanDMXSequenceID getSequenceId() const { return getComponentId(); }

	static const std::string k_groupIdPropertyId;
	inline MikanDMXFixtureGroupID getGroupId() const { return m_groupId; }
	void setGroupId(MikanDMXFixtureGroupID groupId);

	static const std::string k_sequenceNamePropertyId;
	inline const std::string& getSequenceName() const { return m_sequenceName; }
	void setSequenceName(const std::string& sequenceName);

	// Zero or less runs until stopped
	static const std::string k_durationSecondsPropertyId;
	inline float getDurationSeconds() const { return m_durationSeconds; }
	void setDurationSeconds(float durationSeconds);

	static const std::string k_loopPropertyId;
	inline bool getLoop() const { return m_bLoop; }
	void setLoop(bool bLoop);

private:
	MikanDMXFixtureGroupID m_groupId= INVALID_MIKAN_ID;
	std::string m_sequenceName;
	float m_durationSeconds= 10.f;
	bool m_bLoop= true;
};

// -- DMXSequenceComponent -----
class DMXSequenceComponent : public MikanComponent
{
public:
	DMXSequenceComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName= "DMXSequenceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline DMXSequenceDefinitionPtr getDMXSequenceDefinition() const
	{
		return std::static_pointer_cast<DMXSequenceDefinition>(m_definition);
	}

	// The group this sequence animates, null when the id resolves to nothing
	DMXFixtureGroupComponentPtr getGroup() const;

	// -- Playback --
	inline eDMXSequenceState getPlaybackState() const { return m_state; }
	inline float getTimeSinceStart() const { return m_timeSinceStart; }
	inline bool isPlaying() const { return m_state == eDMXSequenceState::Playing; }
	// From Stopped: restart at zero and call the handler's start; from Paused: resume
	void play();
	void pause();
	// Calls the handler's stop and leaves the last frame on the fixtures
	void stop();
	// Advance a playing sequence by one frame: time, the handler's update, then
	// the frame buffer to the fixtures. Called by the owning system.
	void tick(float deltaSeconds);

	// Advance time by delta against a duration (zero or less is unbounded).
	// Looping wraps time; otherwise time clamps to the duration and outFinished
	// is set so the caller runs a final update and stops.
	static void advanceTime(float& inoutTime, float delta, float duration, bool bLoop, bool& outWrapped,
							bool& outFinished);

	// -- Frame buffer, written by the handler and applied after each update --
	inline const DMXFixtureValueMap& getFrameBuffer() const { return m_frameBuffer; }
	bool getFrameBufferValues(MikanLightID fixtureId, std::vector<uint8_t>& outValues) const;
	void setFixtureChannels(MikanLightID fixtureId, const std::vector<uint8_t>& values);
	void setFixtureColor(MikanLightID fixtureId, uint8_t r, uint8_t g, uint8_t b);
	// Ignored unless the fixture is a pixel grid member and the pixel is in range
	void setPixel(MikanLightID fixtureId, int col, int row, uint8_t r, uint8_t g, uint8_t b);
	// Every member: spot lights get one triple, pixel grids every pixel
	void fillGroup(uint8_t r, uint8_t g, uint8_t b);

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static const std::string k_playbackStatePropertyId;
	static const std::string k_timeSinceStartPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_playFunctionId;
	static const std::string k_pauseFunctionId;
	static const std::string k_stopFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(const std::string& functionName) override;

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

private:
	CommonScriptContextPtr getScriptContext() const;
	// Call one handler field with (sequence[, time, delta]); false on a Lua
	// error, which is logged with the sequence and script names
	bool callHandler(const char* field, bool bWithTime, float deltaSeconds);
	void applyFrameBuffer();

	eDMXSequenceState m_state= eDMXSequenceState::Stopped;
	float m_timeSinceStart= 0.f;
	DMXFixtureValueMap m_frameBuffer;
};
