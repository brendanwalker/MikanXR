#pragma once

#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"

#include <functional>
#include <memory>
#include <map>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class IMikanEventFactory
{
public:
	virtual MikanEventPtr createEvent(json jsonEvent) = 0;
};
using IMikanEventFactoryPtr = std::shared_ptr<IMikanEventFactory>;

template <typename t_response_type>
class MikanEventFactoryTyped : public IMikanEventFactory
{
public:
	virtual MikanEventPtr createEvent(json jsonEvent) override
	{
		auto eventPtr = std::make_shared<t_response_type>();

		from_json(jsonEvent, *eventPtr);

		return eventPtr;
	}
};

class MikanEventManager
{
public:
	MikanEventManager() = default;

	template <typename t_response_type>
	void addEventFactory()
	{
		IMikanEventFactoryPtr factory = std::make_shared<MikanEventFactoryTyped<t_response_type>>();
		m_eventFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	template <typename t_response_type>
	void addRefurekuEventFactory()
	{
		IMikanEventFactoryPtr factory = std::make_shared<MikanEventFactoryTyped<t_response_type>>();
		m_eventFactories.insert(std::make_pair(t_response_type::k_typeName, factory));
	}

	MikanResult fetchNextEvent(MikanEventPtr& out_event);

protected:
	MikanEventPtr parseEventString(const char* utf8EventString);

private:
	std::map<std::string, IMikanEventFactoryPtr> m_eventFactories;
};