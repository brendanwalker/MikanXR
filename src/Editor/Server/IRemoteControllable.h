#pragma once

#include <string>
#include <vector>

class IRemoteControllable
{
public:
	virtual ~IRemoteControllable() {}

	inline void setRemoteControlManager(class RemoteControlManager* pRemoteControlManager)
	{
		m_pRemoteControlManager= pRemoteControlManager;
	}

	void sendRemoteControlEvent(const std::string& event);
	void sendRemoteControlEvent(
		const std::string& event,
		const std::vector<std::string>& parameters);

	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults);

	static const std::string k_success;
	static const std::string k_failure;
	static const std::string k_true;
	static const std::string k_false;

private:
	class RemoteControlManager* m_pRemoteControlManager= nullptr;
};