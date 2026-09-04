#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "HttpInterprocessMessageServer.h"
#include "ScriptRequestHandler.h"
#include "Logger.h"
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
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Script Requests
	messageServer->setRequestHandler(InvokeScriptTrigger::staticGetArchetype().getName(),
									 std::bind(&ScriptRequestHandler::invokeScriptTriggerHandler, this, _1, _2));
	messageServer->setRequestHandler(SendScriptMessage::staticGetArchetype().getName(),
									 std::bind(&ScriptRequestHandler::invokeScriptMessageHandler, this, _1, _2));

	return true;
}

void ScriptRequestHandler::shutdown() {}

void ScriptRequestHandler::bindScriptContext(CommonScriptContextPtr scriptContext)
{
	if (CommonScriptContextPtr boundContext= m_scriptContext.lock())
	{
		unbindScriptContext(boundContext);
	}

	m_scriptContext= scriptContext;
	scriptContext->OnScriptMessage+= MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);

	// Register any HTTP trigger routes the scripts declared via ScriptContext.registerHttpTrigger(...)
	for (const auto& binding : scriptContext->getHttpTriggerBindings())
	{
		registerHttpTriggerRoute(binding.routeName, binding.triggerName);
	}
}

void ScriptRequestHandler::unbindScriptContext(CommonScriptContextPtr scriptContext)
{
	if (m_scriptContext.lock() != scriptContext)
		return;

	scriptContext->OnScriptMessage-= MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);
	m_scriptContext.reset();

	for (const auto& binding : scriptContext->getHttpTriggerBindings())
	{
		unregisterHttpTriggerRoute(binding.routeName);
	}
}

bool ScriptRequestHandler::registerHttpTriggerRoute(const std::string& routeName, const std::string& triggerName)
{
	HttpInterprocessMessageServer* httpServer= m_owner->getHttpMessageServer();
	if (!httpServer)
	{
		return false;
	}

	auto handler= [this, triggerName](const std::string& method, const std::string& path,
									  const std::string& body) -> HttpRouteResponse
	{
		MikanAPIResult result= invokeScriptTriggerInternal(triggerName);

		HttpRouteResponse response;
		switch (result)
		{
		case MikanAPIResult::Success:
			response.statusCode= 200;
			response.body= "{\"resultCode\":\"Success\"}";
			break;
		case MikanAPIResult::MalformedParameters:
			response.statusCode= 400;
			response.body= "{\"resultCode\":\"MalformedParameters\"}";
			break;
		case MikanAPIResult::RequestFailed:
			response.statusCode= 422;
			response.body= "{\"resultCode\":\"RequestFailed\"}";
			break;
		default:
			response.statusCode= 500;
			response.body= "{\"resultCode\":\"GeneralError\"}";
			break;
		}

		return response;
	};

	return httpServer->setRouteHandler("/trigger/" + routeName, handler);
}

void ScriptRequestHandler::unregisterHttpTriggerRoute(const std::string& routeName)
{
	HttpInterprocessMessageServer* httpServer= m_owner->getHttpMessageServer();
	if (httpServer)
	{
		httpServer->removeRouteHandler("/trigger/" + routeName);
	}
}

// Scripting Events
void ScriptRequestHandler::publishScriptMessageEvent(const std::string& message)
{
	MikanScriptMessagePostedEvent messageInfo;
	messageInfo.message= message.c_str();

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(messageInfo));
}

// Scripting Requests
void ScriptRequestHandler::invokeScriptTriggerHandler(const ClientRequest& request, ClientResponse& response)
{
	InvokeScriptTrigger scriptTriggerRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptTriggerRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanAPIResult result= invokeScriptTriggerInternal(scriptTriggerRequest.trigger_name.getUtf8Value());

	writeSimpleJsonResponse(request.requestId, result, response);
}

MikanAPIResult ScriptRequestHandler::invokeScriptTriggerInternal(const std::string& triggerName)
{
	CommonScriptContextPtr scriptContext= m_scriptContext.lock();
	if (!scriptContext)
	{
		return MikanAPIResult::RequestFailed;
	}

	if (!scriptContext->hasTrigger(triggerName))
	{
		return MikanAPIResult::MalformedParameters;
	}

	if (!scriptContext->invokeScriptTrigger(triggerName))
	{
		return MikanAPIResult::RequestFailed;
	}

	return MikanAPIResult::Success;
}

void ScriptRequestHandler::invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response)
{
	SendScriptMessage scriptMessageRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptMessageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Offer the message to the project's script handlers; an unhandled message is not an error
	if (CommonScriptContextPtr scriptContext= m_scriptContext.lock())
	{
		scriptContext->invokeScriptMessageHandler(scriptMessageRequest.message.content.getUtf8Value());
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}
