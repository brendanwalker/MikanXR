#pragma once

#include <string>
#include <vector>

class IRemoteControllableAppStage
{
public:
	virtual ~IRemoteControllableAppStage() {}

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
		const std::vector<std::string>& parameters);

private:
	class RemoteControlManager* m_pRemoteControlManager= nullptr;
};

class RemoteControllableAppStageFactory
{
public:
	virtual std::string getAppStageName() = 0;
	virtual IRemoteControllableAppStage* pushAppStage() = 0;
};