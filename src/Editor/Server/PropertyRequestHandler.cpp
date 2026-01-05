#include "App.h"
#include "AppStage.h"
#include "CommonScriptContext.h"
#include "MainWindow.h"
#include "MikanComponent.h"
#include "MikanClientConnectionState.h"
#include "MikanObjectSystem.h"
#include "MikanServer.h"
#include "MikanPropertyDatabase.h"
#include "PropertyDatabaseEnumerator.h"
#include "PropertyRequestHandler.h"
#include "MikanPropertyEvents.h"
#include "MikanPropertyRequests.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

// -- PropertyRequestHandler -- //
bool PropertyRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Script Requests	
	messageServer->setRequestHandler(
		PropertySetValueRequest::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::setPropertyValueHandler, this, _1, _2));
	messageServer->setRequestHandler(
		PropertyGetValueRequest::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::getPropertyValueHandler, this, _1, _2));
	messageServer->setRequestHandler(
		SetPropertyNotifyMode::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::setPropertyNotifyModeHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetPropertyDescriptors::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::getPropertyDescriptorsHandler, this, _1, _2));

	return true;
}

void PropertyRequestHandler::shutdown()
{
}

// Scripting Events
void PropertyRequestHandler::setPropertyValueHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	PropertySetValueRequest setValueRequest;
	if (!readTypedRequest(request.utf8RequestString, setValueRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = setValueRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& propertyValueName = setValueRequest.fieldName.getValue();
	const MikanVariant& variantValue = setValueRequest.fieldValue;

	MikanPropertyDatabaseConstPtr propertyDatabase = getProjectManager()->getPropertyDatabaseConst();
	if (setValueRequest.componentId != -1)
	{
		MikanComponentPtr componentPtr = objectSystem->getComponentById(setValueRequest.componentId);
		if (!componentPtr)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		RmlPropertyDescriptorConstPtr propertyDescriptor =
			propertyDatabase->findPropertyDescriptor(
				setValueRequest.ownerSystem.getValue(),
				componentPtr->getComponentClassName(),
				setValueRequest.fieldName.getValue());
		if (!propertyDescriptor)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Set the property value on the component
		if (!componentPtr->setPropertyValueFromRml(propertyDescriptor, variantValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}
	else
	{
		RmlPropertyDescriptorConstPtr propertyDescriptor =
			propertyDatabase->findPropertyDescriptor(
				setValueRequest.ownerSystem.getValue(),
				"",
				setValueRequest.fieldName.getValue());
		if (!propertyDescriptor)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Set the property value on the system
		if (!objectSystem->setPropertyValueFromRml(propertyDescriptor, variantValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}

	// Return a successful response
	PropertySetValueResponse setValueResponse;
	writeTypedJsonResponse(request.requestId, setValueResponse, response);
}

void PropertyRequestHandler::getPropertyValueHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	PropertyGetValueRequest getValueRequest;
	if (!readTypedRequest(request.utf8RequestString, getValueRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = getValueRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Build the response
	PropertyGetValueResponse getValueResponse = {};
	getValueResponse.propertyValue.ownerSystem = getValueRequest.ownerSystem;
	getValueResponse.propertyValue.componentId = getValueRequest.componentId;
	getValueResponse.propertyValue.fieldName = getValueRequest.fieldName;

	MikanPropertyDatabaseConstPtr propertyDatabase = getProjectManager()->getPropertyDatabaseConst();
	if (getValueRequest.componentId != -1)
	{
		MikanComponentPtr componentPtr = objectSystem->getComponentById(getValueRequest.componentId);
		if (!componentPtr)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		RmlPropertyDescriptorConstPtr propertyDescriptor =
			propertyDatabase->findPropertyDescriptor(
				getValueRequest.ownerSystem.getValue(),
				componentPtr->getComponentClassName(),
				getValueRequest.fieldName.getValue());
		if (!propertyDescriptor)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Get the property value from the component
		if (!componentPtr->getPropertyValueFromRml(propertyDescriptor, getValueResponse.propertyValue.fieldValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}
	else
	{
		RmlPropertyDescriptorConstPtr propertyDescriptor =
			propertyDatabase->findPropertyDescriptor(
				getValueRequest.ownerSystem.getValue(),
				"", // No component class name for system properties
				getValueRequest.fieldName.getValue());
		if (!propertyDescriptor)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Get the property value from the system
		if (!objectSystem->getPropertyValueFromRml(propertyDescriptor, getValueResponse.propertyValue.fieldValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}


	writeTypedJsonResponse(request.requestId, getValueResponse, response);
}

void PropertyRequestHandler::setPropertyNotifyModeHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SetPropertyNotifyMode setPropertyNotifyMode;
	if (!readTypedRequest(request.utf8RequestString, setPropertyNotifyMode))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = m_owner->getConnectedClientState(request.connectionId);
	if (!clientState)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	if (!clientState->setPropertyNotifyMode(
		setPropertyNotifyMode.systemFilter.getValue(),
		setPropertyNotifyMode.componentFilter.getValue(),
		setPropertyNotifyMode.propertyFilter.getValue(),
		setPropertyNotifyMode.notifyMode))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidParam, response);
		return;
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void PropertyRequestHandler::getPropertyDescriptorsHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetPropertyDescriptors getDescriptorsRequest;
	if (!readTypedRequest(request.utf8RequestString, getDescriptorsRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	PropertyDescriptorResponse propertyDescriptorResponse;

	MikanPropertyDatabaseConstPtr propertyDatabase = getProjectManager()->getPropertyDatabaseConst();
	PropertyDatabaseEnumerator enumerator(
		propertyDatabase,
		getDescriptorsRequest.systemFilter.getValue(),
		getDescriptorsRequest.componentFilter.getValue(),
		getDescriptorsRequest.propertyFilter.getValue());
	while (enumerator.isValid())
	{
		int propertyIndex = enumerator.getCurrentPropertyIndex();
		const MikanPropertyEntry* propertyEntry = propertyDatabase->getPropertyByIndex(propertyIndex);

		MikanPropertyDescriptor descriptorResult = {};
		descriptorResult.ownerSystemClass.setValue(propertyEntry->systemName);
		descriptorResult.ownerComponentClass.setValue(propertyEntry->componentClassName);
		descriptorResult.fieldName.setValue(propertyEntry->descriptor->getName());
		descriptorResult.fieldType = propertyEntry->descriptor->getDataType();
		descriptorResult.isReadOnly = propertyEntry->descriptor->isReadOnly();

		propertyDescriptorResponse.descriptor_list.push_back(descriptorResult);

		enumerator.next();
	}

	writeTypedJsonResponse(request.requestId, propertyDescriptorResponse, response);
}