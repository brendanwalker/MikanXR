#include "IRemoteControllable.h"
#include "RemoteControlManager.h"

const std::string IRemoteControllable::k_success = "success";
const std::string IRemoteControllable::k_failure = "failure";
const std::string IRemoteControllable::k_true = "true";
const std::string IRemoteControllable::k_false = "false";

void IRemoteControllable::sendRemoteControlEvent(const std::string& event)
{
	if (m_pRemoteControlManager)
	{
		std::vector<std::string> noParameters;

		m_pRemoteControlManager->sendRemoteControlEvent(event, noParameters);
	}
}

void IRemoteControllable::sendRemoteControlEvent(
	const std::string& event,
	const std::vector<std::string>& parameters)
{
	if (m_pRemoteControlManager)
	{
		m_pRemoteControlManager->sendRemoteControlEvent(event, parameters);
	}
}

bool IRemoteControllable::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	// by default, we don't handle any commands
	return false;
}