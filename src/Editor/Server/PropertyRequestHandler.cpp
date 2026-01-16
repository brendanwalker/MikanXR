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
#include "ServerEntitySerializer.h"

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
	messageServer->setRequestHandler(
		ComponentGetValuesRequest::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::getComponentValuesHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetComponentListRequest::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::getComponentListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		SystemGetValuesRequest::staticGetArchetype().getId(),
		std::bind(&PropertyRequestHandler::getSystemValuesHandler, this, _1, _2));

	return true;
}

void PropertyRequestHandler::shutdown()
{
}

// Property Request Handlers
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

	if (setValueRequest.componentId != -1)
	{
		MikanComponentPtr componentPtr = objectSystem->getComponentById(setValueRequest.componentId);
		if (!componentPtr)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Set the property value on the component
		if (!componentPtr->setPropertyValue(propertyValueName, variantValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}
	else
	{
		// Set the property value on the system
		if (!objectSystem->setPropertyValue(propertyValueName, variantValue))
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

	const std::string& propertyValueName = getValueRequest.fieldName.getValue();
	if (getValueRequest.componentId != -1)
	{
		MikanComponentPtr componentPtr = objectSystem->getComponentById(getValueRequest.componentId);
		if (!componentPtr)
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}

		// Get the property value from the component
		if (!componentPtr->getPropertyValue(propertyValueName, getValueResponse.propertyValue.fieldValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}
	else
	{
		// Get the property value from the system
		if (!objectSystem->getPropertyValue(propertyValueName, getValueResponse.propertyValue.fieldValue))
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
			return;
		}
	}

	writeTypedJsonResponse(request.requestId, getValueResponse, response);
}

void PropertyRequestHandler::getComponentValuesHandler(const ClientRequest& request, ClientResponse& response)
{
	ComponentGetValuesRequest componentValuesRequest;
	if (!readTypedRequest(request.utf8RequestString, componentValuesRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = componentValuesRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanComponentPtr componentPtr = objectSystem->getComponentById(componentValuesRequest.componentId);
	if (!componentPtr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	rfk::Struct const* valuesStruct = componentPtr->getClientAPIValuesStructType();
	if (!valuesStruct)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Build the response
	ComponentGetValuesResponse getValuesResponse = {};
	getValuesResponse.ownerSystem = componentValuesRequest.ownerSystem;
	getValuesResponse.componentClassName = componentPtr->getComponentClassName();

	// Extract the values into the response polymorphic object
	if (!Serialization::serializeFromEntity(
			std::static_pointer_cast<IEntityAccessor>(objectSystem),
			getValuesResponse.valuesObject.allocateByType(valuesStruct),
			*valuesStruct))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	writeTypedJsonResponse(request.requestId, getValuesResponse, response);
}

void PropertyRequestHandler::getComponentListHandler(const ClientRequest& request, ClientResponse& response)
{
	GetComponentListRequest getComponentListRequest;
	if (!readTypedRequest(request.utf8RequestString, getComponentListRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = getComponentListRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Build the response
	ComponentListResponse componentListResponse = {};
	const std::string& componentClassName = getComponentListRequest.componentClassName.getValue();
	if (!objectSystem->getComponentIdList(componentClassName, componentListResponse.componentIdList))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	writeTypedJsonResponse(request.requestId, componentListResponse, response);
}

void PropertyRequestHandler::getSystemValuesHandler(const ClientRequest& request, ClientResponse& response)
{
	ComponentGetValuesRequest componentValuesRequest;
	if (!readTypedRequest(request.utf8RequestString, componentValuesRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& ownerSystemName = componentValuesRequest.ownerSystem.getValue();
	MikanObjectSystemPtr objectSystem = getProjectManager()->getSystemByName(ownerSystemName);
	if (!objectSystem)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	rfk::Struct const* valuesStruct = objectSystem->getClientAPIValuesStructType();
	if (!valuesStruct)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Build the response
	SystemGetValuesResponse getValuesResponse = {};
	getValuesResponse.ownerSystem = componentValuesRequest.ownerSystem;

	// Extract the values into the response polymorphic object
	if (!Serialization::serializeFromEntity(
			std::static_pointer_cast<IEntityAccessor>(objectSystem),
			getValuesResponse.valuesObject.allocateByType(valuesStruct),
			*valuesStruct))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	writeTypedJsonResponse(request.requestId, getValuesResponse, response);
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