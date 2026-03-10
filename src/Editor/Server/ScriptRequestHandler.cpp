#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "ComponentScriptContext.h"
#include "ScriptRequestHandler.h"
#include "MainWindow.h"
#include "MikanComponent.h"
#include "MikanObjectSystem.h"
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
		InvokeComponentScriptTrigger::staticGetArchetype().getId(),
		std::bind(&ScriptRequestHandler::invokeComponentScriptTriggerHandler, this, _1, _2));
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
void ScriptRequestHandler::invokeComponentScriptTriggerHandler(const ClientRequest& request, ClientResponse& response)
{
	InvokeComponentScriptTrigger scriptTriggerRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptTriggerRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = scriptTriggerRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanComponentPtr componentPtr = objectSystem->getComponentById(scriptTriggerRequest.componentId);
	if (!componentPtr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	ComponentScriptContextPtr scriptContext = componentPtr->getScriptContext();
	if (!scriptContext)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::RequestFailed, response);
		return;
	}

	if (!scriptContext->invokeScriptTrigger(scriptTriggerRequest.trigger_name.getValue()))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::RequestFailed, response);
		return;
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

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
