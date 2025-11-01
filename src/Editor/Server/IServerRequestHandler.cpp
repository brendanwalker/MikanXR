#include "IServerRequestHandler.h"
#include "MikanServer.h"

ProjectConfigPtr IServerRequestHandler::getProjectConfig()
{
	return m_owner->getProjectConfig();
}
