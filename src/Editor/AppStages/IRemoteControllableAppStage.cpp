#include "IRemoteControllableAppStage.h"
#include "RemoteControlManager.h"

void IRemoteControllableAppStage::sendRemoteControlEvent(const std::string& event)
{
	if (m_pRemoteControlManager)
	{
		std::vector<std::string> noParameters;

		m_pRemoteControlManager->sendRemoteControlEvent(event, noParameters);
	}
}

void IRemoteControllableAppStage::sendRemoteControlEvent(
	const std::string& event,
	const std::vector<std::string>& parameters)
{
	if (m_pRemoteControlManager)
	{
		m_pRemoteControlManager->sendRemoteControlEvent(event, parameters);
	}
}

bool IRemoteControllableAppStage::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	// by default, we don't handle any commands
	return false;
}