#include "InterprocessMessages.h"
#include "Logger.h"

#include <assert.h>

//-- MikanRemoteFunctionCall -----
uint32_t MikanRemoteFunctionCall::m_nextRequestId= 0;

MikanRemoteFunctionCall::MikanRemoteFunctionCall()
{
	m_header.clientId[0]= '\0';
	m_header.functionName[0]= '\0';
	m_header.requestId= m_nextRequestId++;
	m_header.parameterBufferSize= 0;
}

MikanRemoteFunctionCall::MikanRemoteFunctionCall(
	const char* clientId, 
	const char* functionName)
{
	m_header.requestId = m_nextRequestId++;
	setClientId(clientId);
	setFunctionName(functionName);
	setParameterBuffer(nullptr, 0);
}

MikanRemoteFunctionCall::MikanRemoteFunctionCall(
	const char* clientId, 
	const char* functionName, 
	uint8_t* buffer, 
	size_t bufferSize)
{
	m_header.requestId = m_nextRequestId++;
	setClientId(clientId);
	setFunctionName(functionName);
	setParameterBuffer(buffer, bufferSize);
}

void MikanRemoteFunctionCall::setClientId(const char* clientId)
{
	const size_t clientIdLength= strlen(clientId);
	assert(clientIdLength < sizeof(m_header.clientId));
	memset(m_header.clientId, 0, sizeof(m_header.clientId));
	strncpy(m_header.clientId, clientId, sizeof(m_header.clientId) - 1);
}

void MikanRemoteFunctionCall::setFunctionName(const char* functionName)
{
	const size_t functionNameLength = strlen(functionName);
	assert(functionNameLength < sizeof(m_header.functionName));
	memset(m_header.functionName, 0, sizeof(m_header.functionName));
	strncpy(m_header.functionName, functionName, sizeof(m_header.functionName) - 1);
}

void MikanRemoteFunctionCall::setParameterBuffer(uint8_t* buffer, size_t bufferSize)
{
	assert(bufferSize <= sizeof(m_parameterBuffer));

	if (buffer != nullptr && bufferSize > 0 && bufferSize <= sizeof(m_parameterBuffer))
	{
		std::memcpy(m_parameterBuffer, buffer, bufferSize);
		m_header.parameterBufferSize= bufferSize;
	}
	else
	{
		m_header.parameterBufferSize = 0;
	}
}

size_t MikanRemoteFunctionCall::getTotalSize() const
{
	return sizeof(m_header) + m_header.parameterBufferSize;
}

//-- MikanRemoteFunctionResult ---- 
MikanRemoteFunctionResult::MikanRemoteFunctionResult()
{
	m_header.resultCode= MikanResult_Success;
	m_header.requestId= 0;
	m_header.resultBufferSize = 0;
}

MikanRemoteFunctionResult::MikanRemoteFunctionResult(
	MikanResult result, 
	uint32_t requestId)
{
	setResultCode(result);
	setRequestId(requestId);
	setResultBuffer(nullptr, 0);
}

MikanRemoteFunctionResult::MikanRemoteFunctionResult(
	MikanResult result,
	uint32_t requestId,
	uint8_t* buffer,
	size_t bufferSize)
{
	setResultCode(result);
	setRequestId(requestId);
	setResultBuffer(buffer, bufferSize);
}

void MikanRemoteFunctionResult::setResultCode(MikanResult result)
{
	m_header.resultCode= result;
}

void MikanRemoteFunctionResult::setRequestId(uint32_t requestId)
{
	m_header.requestId= requestId;
}

void MikanRemoteFunctionResult::setResultBuffer(uint8_t* buffer, size_t bufferSize)
{
	assert(bufferSize <= sizeof(m_resultBuffer));

	if (buffer != nullptr && bufferSize > 0 && bufferSize <= sizeof(m_resultBuffer))
	{
		std::memcpy(m_resultBuffer, buffer, bufferSize);
		m_header.resultBufferSize = bufferSize;
	}
	else
	{
		m_header.resultBufferSize = 0;
	}
}

size_t MikanRemoteFunctionResult::getTotalSize() const
{
	return sizeof(m_header) + m_header.resultBufferSize;
}
