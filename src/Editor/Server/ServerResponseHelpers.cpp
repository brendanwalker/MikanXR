#include "ServerResponseHelpers.h"

void writeSimpleJsonResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		Serialization::serializeToJsonString(mikanResponse, response.utf8String);
	}
	else
	{
		response.utf8String = "";
	}
}

void writeSimpleBinaryResponse(MikanRequestID requestId, MikanAPIResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		Serialization::serializeToBytes<MikanResponse>(mikanResponse, response.binaryData);
	}
	else
	{
		response.binaryData.clear();
	}
}

bool extractEntityValues(
	IEntityAccessorPtr entityAccessor,
	const rfk::Struct& valuesStruct,
	Serialization::PolymorphicObjectPtr& outValuesObject)
{
	outValuesObject.allocateByType(&valuesStruct);

	// For some reason Refureku doesn't return fields in the order they were declared
	// So we extract fields into a vector and sort them by memory offset
	using FieldList = std::vector<rfk::Field const*>;
	FieldList sortedFields;
	valuesStruct.foreachField([](rfk::Field const& field, void* userData) -> bool {
		FieldList* sortedFieldsPtr = reinterpret_cast<FieldList*>(userData);
		sortedFieldsPtr->push_back(&field);
		return true;
		}, &sortedFields);

	std::sort(sortedFields.begin(), sortedFields.end(),
		[](rfk::Field const* a, rfk::Field const* b) {
			return a->getMemoryOffset() < b->getMemoryOffset();
		});

	// Emit the fields
	for (rfk::Field const* field : sortedFields)
	{
		const std::string fieldName = field->getName();
		MikanVariant fieldValue;

		// TODO
		//if (!entityAccessor->getPropertyValue(fieldName, fieldValue))
		//{
		//	return false;
		//}
	}

	return true;
}
