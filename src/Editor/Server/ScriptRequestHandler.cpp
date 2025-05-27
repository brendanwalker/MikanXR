#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "ScriptRequestHandler.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanScriptEvents.h"
#include "MikanScriptRequests.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- ScriptRequestHandler -- //
bool ScriptRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Script Requests	
	messageServer->setRequestHandler(
		SendScriptMessage::staticGetArchetype().getId(),
		std::bind(&ScriptRequestHandler::invokeScriptMessageHandler, this, _1, _2));

	return true;
}

void ScriptRequestHandler::shutdown()
{
}

void ScriptRequestHandler::bindScriptContect(CommonScriptContextPtr scriptContext)
{
	m_scriptContexts.push_back(scriptContext);
	scriptContext->OnScriptMessage += MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);
}

void ScriptRequestHandler::unbindScriptContect(CommonScriptContextPtr scriptContext)
{
	for (auto it = m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr scriptContext = it->lock();

		if (scriptContext == scriptContext)
		{
			m_scriptContexts.erase(it);
			scriptContext->OnScriptMessage -= MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);
			break;
		}
	}
}

// Scripting Events
void ScriptRequestHandler::publishScriptMessageEvent(const std::string& message)
{
	MikanScriptMessagePostedEvent messageInfo;
	messageInfo.message = message;

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(messageInfo));
}

// Scripting Requests
void ScriptRequestHandler::invokeScriptMessageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SendScriptMessage scriptMessageRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptMessageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Find the first script context that cares about the message
	for (auto it = m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr scriptContext = it->lock();

		if (scriptContext == scriptContext)
		{
			if (scriptContext->invokeScriptMessageHandler(scriptMessageRequest.message.content.getValue()))
			{
				break;
			}
		}
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}
