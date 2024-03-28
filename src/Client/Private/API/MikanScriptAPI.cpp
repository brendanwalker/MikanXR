
#include "MikanScriptAPI.h"
#include "MikanRequestManager.h"

MikanScriptAPI::MikanScriptAPI(MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
}

MikanResponseFuture MikanScriptAPI::sendScriptMessage(const std::string& mesg)
{
	return m_requestManager->sendRequestWithPayload<std::string>(k_sendScriptMessage, mesg);
}