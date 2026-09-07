#include "DMXSequenceComponent.h"
#include "ProjectScriptContext.h"
#include "DMXFixtureGroupComponent.h"
#include "DMXFixtureGroupSystem.h"
#include "EnumPropertyMetaData.h"
#include "FunctionInterface.h"
#include "Logger.h"
#include "MikanObjectSystem.h"
#include "MikanVariantTypes.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "ScriptObjectSystem.h"

#include <algorithm>
#include <cmath>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

namespace
{
const std::string k_sequenceStateStrings[]= {
	"propertyValues.sequence_stopped",
	"propertyValues.sequence_playing",
	"propertyValues.sequence_paused",
};
}

// -- DMXSequenceDefinition -----
const std::string DMXSequenceDefinition::k_groupIdPropertyId= "group_id";
const std::string DMXSequenceDefinition::k_sequenceNamePropertyId= "sequence_name";
const std::string DMXSequenceDefinition::k_durationSecondsPropertyId= "duration_seconds";
const std::string DMXSequenceDefinition::k_loopPropertyId= "loop";

DMXSequenceDefinition::DMXSequenceDefinition()
	: MikanComponentDefinition()
{
}

DMXSequenceDefinition::DMXSequenceDefinition(MikanDMXSequenceID sequenceId)
	: MikanComponentDefinition(sequenceId, "")
{
}

configuru::Config DMXSequenceDefinition::writeToJSON()
{
	configuru::Config pt= MikanComponentDefinition::writeToJSON();

	pt[k_groupIdPropertyId]= m_groupId;
	pt[k_sequenceNamePropertyId]= m_sequenceName;
	pt[k_durationSecondsPropertyId]= m_durationSeconds;
	pt[k_loopPropertyId]= m_bLoop;

	return pt;
}

void DMXSequenceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_groupId= pt.get_or<MikanDMXFixtureGroupID>(k_groupIdPropertyId, m_groupId);
	m_sequenceName= pt.get_or<std::string>(k_sequenceNamePropertyId, m_sequenceName);
	m_durationSeconds= pt.get_or<float>(k_durationSecondsPropertyId, m_durationSeconds);
	m_bLoop= pt.get_or<bool>(k_loopPropertyId, m_bLoop);
}

bool DMXSequenceDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
											   const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!MikanComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* values= initParams.getTypedPointer<MikanDMXSequenceComponentValues>();
	if (values)
	{
		m_groupId= values->group_id;
		m_sequenceName= values->sequence_name.getUtf8Value();
		m_durationSeconds= values->duration_seconds;
		m_bLoop= values->loop;
	}

	if (m_groupId == INVALID_MIKAN_ID)
	{
		// If no group was specified, use the first one
		auto groupSystem= ownerObjectSystem->getObjectSystemOfType<DMXFixtureGroupSystem>();

		m_groupId= groupSystem->getFirstComponentId();
	}

	return true;
}

void DMXSequenceDefinition::setGroupId(MikanDMXFixtureGroupID groupId)
{
	if (groupId != m_groupId)
	{
		m_groupId= groupId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_groupIdPropertyId));
	}
}

void DMXSequenceDefinition::setSequenceName(const std::string& sequenceName)
{
	if (sequenceName != m_sequenceName)
	{
		m_sequenceName= sequenceName;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_sequenceNamePropertyId));
	}
}

void DMXSequenceDefinition::setDurationSeconds(float durationSeconds)
{
	if (durationSeconds != m_durationSeconds)
	{
		m_durationSeconds= durationSeconds;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_durationSecondsPropertyId));
	}
}

void DMXSequenceDefinition::setLoop(bool bLoop)
{
	if (bLoop != m_bLoop)
	{
		m_bLoop= bLoop;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_loopPropertyId));
	}
}

// -- DMXSequenceComponent -----
const std::string DMXSequenceComponent::k_playbackStatePropertyId= "playback_state";
const std::string DMXSequenceComponent::k_timeSinceStartPropertyId= "time_since_start";
const std::string DMXSequenceComponent::k_playFunctionId= "play_sequence";
const std::string DMXSequenceComponent::k_pauseFunctionId= "pause_sequence";
const std::string DMXSequenceComponent::k_stopFunctionId= "stop_sequence";

DMXSequenceComponent::DMXSequenceComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
}

DMXFixtureGroupComponentPtr DMXSequenceComponent::getGroup() const
{
	// A component built without an owner (the unit tests) has no systems to ask
	if (!getOwnerObject())
		return nullptr;

	DMXFixtureGroupSystemPtr groupSystem= getObjectSystemOfType<DMXFixtureGroupSystem>();
	if (!groupSystem)
		return nullptr;

	return groupSystem->getGroupById(getDMXSequenceDefinition()->getGroupId());
}

CommonScriptContextPtr DMXSequenceComponent::getScriptContext() const
{
	ScriptObjectSystemPtr scriptSystem= getObjectSystemOfType<ScriptObjectSystem>();
	return scriptSystem ? scriptSystem->getScriptContext() : nullptr;
}

void DMXSequenceComponent::play()
{
	if (m_state == eDMXSequenceState::Playing)
		return;

	if (m_state == eDMXSequenceState::Stopped)
	{
		m_timeSinceStart= 0.f;
		m_frameBuffer.clear();
		if (!callHandler("start", false, 0.f))
			return;
		applyFrameBuffer();
	}

	m_state= eDMXSequenceState::Playing;
}

void DMXSequenceComponent::pause()
{
	if (m_state == eDMXSequenceState::Playing)
	{
		m_state= eDMXSequenceState::Paused;
	}
}

void DMXSequenceComponent::stop()
{
	if (m_state == eDMXSequenceState::Stopped)
		return;

	// The last frame stays on the fixtures unless the handler writes another
	m_state= eDMXSequenceState::Stopped;
	m_timeSinceStart= 0.f;
	if (callHandler("stop", false, 0.f))
	{
		applyFrameBuffer();
	}
}

void DMXSequenceComponent::advanceTime(float& inoutTime, float delta, float duration, bool bLoop, bool& outWrapped,
									   bool& outFinished)
{
	outWrapped= false;
	outFinished= false;

	inoutTime+= delta;
	if (duration <= 0.f || inoutTime < duration)
		return;

	if (bLoop)
	{
		inoutTime= std::fmod(inoutTime, duration);
		outWrapped= true;
	}
	else
	{
		inoutTime= duration;
		outFinished= true;
	}
}

void DMXSequenceComponent::tick(float deltaSeconds)
{
	if (m_state != eDMXSequenceState::Playing)
		return;

	// A handler that vanished with a script reload ends the sequence
	CommonScriptContextPtr scriptContext= getScriptContext();
	if (!scriptContext || !scriptContext->hasSequence(getDMXSequenceDefinition()->getSequenceName()))
	{
		m_state= eDMXSequenceState::Stopped;
		m_timeSinceStart= 0.f;
		return;
	}

	DMXSequenceDefinitionPtr definition= getDMXSequenceDefinition();
	bool bWrapped= false;
	bool bFinished= false;
	advanceTime(m_timeSinceStart, deltaSeconds, definition->getDurationSeconds(), definition->getLoop(), bWrapped,
				bFinished);

	if (!callHandler("update", true, deltaSeconds))
		return;
	applyFrameBuffer();

	if (bFinished)
	{
		stop();
	}
}

bool DMXSequenceComponent::callHandler(const char* field, bool bWithTime, float deltaSeconds)
{
	CommonScriptContextPtr scriptContext= getScriptContext();
	if (!scriptContext)
		return false;

	const std::string& sequenceName= getDMXSequenceDefinition()->getSequenceName();
	std::string error;
	const bool bSuccess= scriptContext->callSequenceHandler(
		sequenceName, field,
		[this, bWithTime, deltaSeconds](lua_State* L) -> int
		{
			// The count has to match what actually landed on the stack, since
			// lua_pcall takes it on faith and a failed push leaves nothing
			// behind. A short count reaches the handler as a missing argument,
			// which fails that sequence alone.
			int argCount= 0;

			if (!luabridge::push(L, this))
				return argCount;
			argCount++;

			if (!bWithTime)
				return argCount;

			if (!luabridge::push(L, m_timeSinceStart))
				return argCount;
			argCount++;

			if (!luabridge::push(L, deltaSeconds))
				return argCount;
			argCount++;

			return argCount;
		},
		error);

	if (!bSuccess)
	{
		MIKAN_LOG_ERROR("DMXSequenceComponent::callHandler")
			<< "Sequence " << getName() << " handler " << sequenceName << "." << field << " failed: " << error;
		m_state= eDMXSequenceState::Stopped;
		m_timeSinceStart= 0.f;
	}

	return bSuccess;
}

void DMXSequenceComponent::applyFrameBuffer()
{
	DMXFixtureGroupComponentPtr group= getGroup();
	if (!group)
		return;

	for (const MikanLightID fixtureId : group->getDMXFixtureGroupDefinition()->getFixtureIds())
	{
		auto it= m_frameBuffer.find(fixtureId);
		if (it == m_frameBuffer.end())
			continue;

		DMXFixtureComponentPtr fixture= group->resolveFixture(fixtureId);
		if (!fixture)
			continue;

		std::vector<uint8_t> values= it->second;
		values.resize(fixture->getDMXFixtureDefinition()->getDMXChannelCount(), 0);
		fixture->setChannelValues(values);
	}
}

bool DMXSequenceComponent::getFrameBufferValues(MikanLightID fixtureId, std::vector<uint8_t>& outValues) const
{
	auto it= m_frameBuffer.find(fixtureId);
	if (it == m_frameBuffer.end())
		return false;

	outValues= it->second;
	return true;
}

void DMXSequenceComponent::setFixtureChannels(MikanLightID fixtureId, const std::vector<uint8_t>& values)
{
	m_frameBuffer[fixtureId]= values;
}

void DMXSequenceComponent::setFixtureColor(MikanLightID fixtureId, uint8_t r, uint8_t g, uint8_t b)
{
	std::vector<uint8_t>& values= m_frameBuffer[fixtureId];
	if (values.size() < 3)
		values.resize(3, 0);
	values[0]= r;
	values[1]= g;
	values[2]= b;
}

void DMXSequenceComponent::setPixel(MikanLightID fixtureId, int col, int row, uint8_t r, uint8_t g, uint8_t b)
{
	DMXFixtureGroupComponentPtr group= getGroup();
	RGBPixelGridComponentPtr pixelGrid=
		group ? std::dynamic_pointer_cast<RGBPixelGridComponent>(group->resolveFixture(fixtureId)) : nullptr;
	if (!pixelGrid)
		return;

	// The grid's origin corner and zig-zag wiring decide where the cell lands in
	// the DMX stream. Out of range cells are refused before the buffer is
	// reached, so nothing is left behind for a fixture nothing was written to.
	RGBPixelGridDefinitionPtr gridDefinition= pixelGrid->getRGBPixelGridDefinition();
	const int wireIndex= gridDefinition->getPixelWireIndex(col, row);
	if (wireIndex < 0)
		return;

	std::vector<uint8_t>& values= m_frameBuffer[fixtureId];
	const size_t channelCount= static_cast<size_t>(gridDefinition->getTotalChannels());
	if (values.size() < channelCount)
		values.resize(channelCount, 0);

	const size_t offset= static_cast<size_t>(wireIndex) * 3;
	if (offset + 3 > values.size())
		return;

	values[offset]= r;
	values[offset + 1]= g;
	values[offset + 2]= b;
}

void DMXSequenceComponent::fillGroup(uint8_t r, uint8_t g, uint8_t b)
{
	DMXFixtureGroupComponentPtr group= getGroup();
	if (!group)
		return;

	for (const MikanLightID fixtureId : group->getDMXFixtureGroupDefinition()->getFixtureIds())
	{
		DMXFixtureComponentPtr fixture= group->resolveFixture(fixtureId);
		if (!fixture)
			continue;

		// One triple repeated across the fixture's channels
		const size_t channelCount= fixture->getDMXFixtureDefinition()->getDMXChannelCount();
		std::vector<uint8_t> values(channelCount, 0);
		for (size_t i= 0; i + 2 < channelCount; i+= 3)
		{
			values[i]= r;
			values[i + 1]= g;
			values[i + 2]= b;
		}
		m_frameBuffer[fixtureId]= values;
	}
}

// -- IEntityAccessor ----
rfk::Struct const* DMXSequenceComponent::getClientAPIValuesStructType() const
{
	return &MikanDMXSequenceComponentValues::staticGetArchetype();
}

// -- IPropertyInterface ----
void DMXSequenceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(DMXSequenceDefinition::k_groupIdPropertyId, MikanVariantType::INT));
	// The panel draws the handler picker itself
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(DMXSequenceDefinition::k_sequenceNamePropertyId, MikanVariantType::STRING)
			->setUIHidden());
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(DMXSequenceDefinition::k_durationSecondsPropertyId,
																  MikanVariantType::FLOAT));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(DMXSequenceDefinition::k_loopPropertyId, MikanVariantType::BOOL));
	// Runtime playback status, drawn by the panel as a status line
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(k_playbackStatePropertyId, MikanVariantType::INT)
								 ->setReadOnly()
								 ->setUIHidden()
								 ->addMetaData(std::make_shared<EnumPropertyMetaData>(k_sequenceStateStrings, 3)));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(k_timeSinceStartPropertyId, MikanVariantType::FLOAT)
								 ->setReadOnly()
								 ->setUIHidden());
}

bool DMXSequenceComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	DMXSequenceDefinitionPtr definition= getDMXSequenceDefinition();

	if (propertyName == DMXSequenceDefinition::k_groupIdPropertyId)
	{
		outValue= static_cast<int>(definition->getGroupId());
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_sequenceNamePropertyId)
	{
		outValue= definition->getSequenceName();
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_durationSecondsPropertyId)
	{
		outValue= definition->getDurationSeconds();
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_loopPropertyId)
	{
		outValue= definition->getLoop();
		return true;
	}
	else if (propertyName == k_playbackStatePropertyId)
	{
		outValue= static_cast<int>(m_state);
		return true;
	}
	else if (propertyName == k_timeSinceStartPropertyId)
	{
		outValue= m_timeSinceStart;
		return true;
	}

	return MikanComponent::getPropertyValue(propertyName, outValue);
}

bool DMXSequenceComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	DMXSequenceDefinitionPtr definition= getDMXSequenceDefinition();

	if (propertyName == DMXSequenceDefinition::k_groupIdPropertyId)
	{
		if (inValue.value_type != MikanVariantType::INT)
			return false;

		definition->setGroupId(inValue.getIntValue());
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_sequenceNamePropertyId)
	{
		if (inValue.value_type != MikanVariantType::STRING)
			return false;

		definition->setSequenceName(inValue.getUtf8Value());
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_durationSecondsPropertyId)
	{
		if (inValue.value_type != MikanVariantType::FLOAT)
			return false;

		definition->setDurationSeconds(inValue.getFloatValue());
		return true;
	}
	else if (propertyName == DMXSequenceDefinition::k_loopPropertyId)
	{
		if (inValue.value_type != MikanVariantType::BOOL)
			return false;

		definition->setLoop(inValue.getBoolValue());
		return true;
	}

	return MikanComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
void DMXSequenceComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<FunctionDescriptor>(k_playFunctionId, "Play"));
	outDescriptors.push_back(std::make_shared<FunctionDescriptor>(k_pauseFunctionId, "Pause"));
	outDescriptors.push_back(std::make_shared<FunctionDescriptor>(k_stopFunctionId, "Stop"));
}

bool DMXSequenceComponent::invokeFunction(const std::string& functionName)
{
	if (functionName == k_playFunctionId)
	{
		play();
		return true;
	}
	else if (functionName == k_pauseFunctionId)
	{
		pause();
		return true;
	}
	else if (functionName == k_stopFunctionId)
	{
		stop();
		return true;
	}

	return MikanComponent::invokeFunction(functionName);
}

// -- Lua Binding ----
void DMXSequenceComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<DMXSequenceComponent, MikanComponent>(DMXSequenceComponent::k_componentClassName.c_str())
		.addProperty("groupId",
					 [](DMXSequenceComponent* c) -> int { return c->getDMXSequenceDefinition()->getGroupId(); })
		.addProperty("sequenceName", [](DMXSequenceComponent* c) -> std::string
					 { return c->getDMXSequenceDefinition()->getSequenceName(); })
		.addProperty("timeSinceStart", [](DMXSequenceComponent* c) -> float { return c->getTimeSinceStart(); })
		.addProperty("isPlaying", [](DMXSequenceComponent* c) -> bool { return c->isPlaying(); })
		.addFunction("getGroup",
					 [](DMXSequenceComponent* c) -> DMXFixtureGroupComponent* { return c->getGroup().get(); })
		.addFunction("play", [](DMXSequenceComponent* c) { c->play(); })
		.addFunction("pause", [](DMXSequenceComponent* c) { c->pause(); })
		.addFunction("stop", [](DMXSequenceComponent* c) { c->stop(); })
		.addFunction("setFixtureColor", [](DMXSequenceComponent* c, int fixtureId, int r, int g, int b)
					 { c->setFixtureColor(fixtureId, (uint8_t)r, (uint8_t)g, (uint8_t)b); })
		.addFunction("setPixel", [](DMXSequenceComponent* c, int fixtureId, int col, int row, int r, int g, int b)
					 { c->setPixel(fixtureId, col, row, (uint8_t)r, (uint8_t)g, (uint8_t)b); })
		.addFunction("setFixtureChannels",
					 [](DMXSequenceComponent* c, int fixtureId, luabridge::LuaRef bytes)
					 {
						 std::vector<uint8_t> values;
						 if (bytes.isTable())
						 {
							 const int count= static_cast<int>(bytes.length());
							 for (int i= 1; i <= count; ++i)
								 values.push_back(
									 static_cast<uint8_t>(std::clamp(bytes[i].unsafe_cast<int>(), 0, 255)));
						 }
						 c->setFixtureChannels(fixtureId, values);
					 })
		.addFunction("fillGroup", [](DMXSequenceComponent* c, int r, int g, int b)
					 { c->fillGroup((uint8_t)r, (uint8_t)g, (uint8_t)b); })
		.endClass();
}
