#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "ComponentScriptContext.h"
#include "HttpInterprocessMessageServer.h"
#include "ScriptRequestHandler.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MikanComponent.h"
#include "MikanObject.h"
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
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Script Requests
	messageServer->setRequestHandler(
		InvokeComponentScriptTrigger::staticGetArchetype().getName(),
		std::bind(&ScriptRequestHandler::invokeComponentScriptTriggerHandler, this, _1, _2));
	messageServer->setRequestHandler(SendScriptMessage::staticGetArchetype().getName(),
									 std::bind(&ScriptRequestHandler::invokeScriptMessageHandler, this, _1, _2));

	return true;
}

void ScriptRequestHandler::shutdown() {}

void ScriptRequestHandler::bindScriptContect(CommonScriptContextPtr scriptContext)
{
	m_scriptContexts.push_back(scriptContext);
	scriptContext->OnScriptMessage+= MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);

	// If this script context is owned by a component, register any HTTP trigger routes
	// the script declared via ScriptContext.registerHttpTrigger(...)
	ComponentScriptContextPtr componentScriptContext= std::dynamic_pointer_cast<ComponentScriptContext>(scriptContext);
	if (componentScriptContext)
	{
		MikanComponentPtr ownerComponent= componentScriptContext->getOwnerComponent();
		if (ownerComponent)
		{
			const std::string ownerSystemName=
				ownerComponent->getOwnerObject()->getOwnerSystem()->getObjectSystemClassName();
			const int componentId= ownerComponent->getComponentId();

			for (const auto& binding : scriptContext->getHttpTriggerBindings())
			{
				registerHttpTriggerRoute(binding.routeName, ownerSystemName, componentId, binding.triggerName);
			}
		}
	}
}

void ScriptRequestHandler::unbindScriptContect(CommonScriptContextPtr scriptContext)
{
	for (auto it= m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr boundContext= it->lock();

		if (boundContext == scriptContext)
		{
			m_scriptContexts.erase(it);
			boundContext->OnScriptMessage-= MakeDelegate(this, &ScriptRequestHandler::publishScriptMessageEvent);
			break;
		}
	}

	// Unregister any HTTP trigger routes this script context had registered
	for (const auto& binding : scriptContext->getHttpTriggerBindings())
	{
		unregisterHttpTriggerRoute(binding.routeName);
	}
}

void ScriptRequestHandler::getBoundScriptContexts(std::vector<CommonScriptContextPtr>& outContexts) const
{
	for (const CommonScriptContextWeakPtr& weakContext : m_scriptContexts)
	{
		CommonScriptContextPtr context= weakContext.lock();
		if (context)
		{
			outContexts.push_back(context);
		}
	}
}

bool ScriptRequestHandler::registerHttpTriggerRoute(const std::string& routeName, const std::string& ownerSystemName,
													int componentId, const std::string& triggerName)
{
	HttpInterprocessMessageServer* httpServer= m_owner->getHttpMessageServer();
	if (!httpServer)
	{
		return false;
	}

	auto handler= [this, ownerSystemName, componentId, triggerName](const std::string& method, const std::string& path,
																	const std::string& body) -> HttpRouteResponse
	{
		MikanAPIResult result= invokeComponentScriptTriggerInternal(ownerSystemName, componentId, triggerName);

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
void ScriptRequestHandler::invokeComponentScriptTriggerHandler(const ClientRequest& request, ClientResponse& response)
{
	InvokeComponentScriptTrigger scriptTriggerRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptTriggerRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanAPIResult result= invokeComponentScriptTriggerInternal(scriptTriggerRequest.ownerSystem.getUtf8Value(),
																scriptTriggerRequest.componentId,
																scriptTriggerRequest.trigger_name.getUtf8Value());

	writeSimpleJsonResponse(request.requestId, result, response);
}

MikanAPIResult ScriptRequestHandler::invokeComponentScriptTriggerInternal(const std::string& ownerSystemName,
																		  int componentId,
																		  const std::string& triggerName)
{
	MikanObjectSystemPtr objectSystem= getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		return MikanAPIResult::MalformedParameters;
	}

	MikanComponentPtr componentPtr= objectSystem->getComponentById(componentId);
	if (!componentPtr)
	{
		return MikanAPIResult::MalformedParameters;
	}

	ComponentScriptContextPtr scriptContext= componentPtr->getScriptContext();
	if (!scriptContext)
	{
		return MikanAPIResult::RequestFailed;
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

	// Find the first script context that cares about the message
	for (auto it= m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr boundContext= it->lock();

		if (boundContext)
		{
			const std::string message= scriptMessageRequest.message.content.getUtf8Value();

			if (boundContext->invokeScriptMessageHandler(message))
			{
				break;
			}
		}
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}
