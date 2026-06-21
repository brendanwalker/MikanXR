#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "MainWindow.h"
#include "MikanComponent.h"
#include "MikanClientConnectionState.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MikanServer.h"
#include "MikanFunctionDatabase.h"
#include "FunctionDatabaseEnumerator.h"
#include "FunctionRequestHandler.h"
#include "MikanFunctionRequests.h"
#include "MulticastDelegate.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- FunctionRequestHandler -- //
bool FunctionRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Function Requests
	messageServer->setRequestHandler(
		InvokeSystemFunctionRequest::staticGetArchetype().getName(),
		std::bind(&FunctionRequestHandler::invokeSystemFunctionRequestHandler, this, _1, _2));
	messageServer->setRequestHandler(
		InvokeComponentFunctionRequest::staticGetArchetype().getName(),
		std::bind(&FunctionRequestHandler::invokeComponentFunctionRequestHandler, this, _1, _2));
	messageServer->setRequestHandler(GetFunctionListRequest::staticGetArchetype().getName(),
									 std::bind(&FunctionRequestHandler::getFunctionListHandler, this, _1, _2));

	return true;
}

// Function Request Handlers
void FunctionRequestHandler::invokeSystemFunctionRequestHandler(const ClientRequest& request, ClientResponse& response)
{
	InvokeComponentFunctionRequest invokeFunctionRequest;
	if (!readTypedRequest(request.utf8RequestString, invokeFunctionRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const char* ownerSystemName= invokeFunctionRequest.ownerSystem.getUtf8Value();
	MikanObjectSystemPtr objectSystem= getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	if (!objectSystem->invokeFunction(invokeFunctionRequest.functionName.getUtf8Value()))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownFunction, response);
		return;
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void FunctionRequestHandler::invokeComponentFunctionRequestHandler(const ClientRequest& request,
																   ClientResponse& response)
{
	InvokeComponentFunctionRequest invokeFunctionRequest;
	if (!readTypedRequest(request.utf8RequestString, invokeFunctionRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const char* ownerSystemName= invokeFunctionRequest.ownerSystem.getUtf8Value();
	MikanObjectSystemPtr objectSystem= getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanComponentPtr componentPtr= objectSystem->getComponentById(invokeFunctionRequest.componentId);
	if (!componentPtr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	if (!componentPtr->invokeFunction(invokeFunctionRequest.functionName.getUtf8Value()))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownFunction, response);
		return;
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void FunctionRequestHandler::getFunctionListHandler(const ClientRequest& request, ClientResponse& response)
{
	GetFunctionListRequest getFunctionListRequest;
	if (!readTypedRequest(request.utf8RequestString, getFunctionListRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	FunctionDescriptorResponse propertyDescriptorResponse;

	MikanFunctionDatabaseConstPtr propertyDatabase= getProjectManager()->getFunctionDatabaseConst();
	FunctionDatabaseEnumerator enumerator(propertyDatabase, getFunctionListRequest.systemFilter.getUtf8Value(),
										  getFunctionListRequest.componentFilter.getUtf8Value(), "");
	while (enumerator.isValid())
	{
		int propertyIndex= enumerator.getCurrentFunctionIndex();
		const MikanFunctionEntry* propertyEntry= propertyDatabase->getFunctionByIndex(propertyIndex);

		MikanFunctionDescriptor descriptorResult= {};
		descriptorResult.ownerSystemClass.setUtf8Value(propertyEntry->systemName.c_str());
		descriptorResult.ownerComponentClass.setUtf8Value(propertyEntry->componentClassName.c_str());
		descriptorResult.functionName.setUtf8Value(propertyEntry->descriptor->getFunctionName().c_str());
		descriptorResult.displayName.setUtf8Value(propertyEntry->descriptor->getDisplayName().c_str());

		propertyDescriptorResponse.descriptor_list.push_back(descriptorResult);

		enumerator.next();
	}

	writeTypedJsonResponse(request.requestId, propertyDescriptorResponse, response);
}