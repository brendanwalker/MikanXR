#include "MikanResponseFuture.h"
#include "MikanRequestManager.h"

struct MikanResponseFutureImpl
{
	MikanRequestManager* ownerRequestManager = nullptr;
	MikanRequestID requestId= INVALID_MIKAN_ID;
	std::future<MikanResponsePtr> future;
};

MikanResponseFuture::MikanResponseFuture() : m_impl(nullptr)
{
}

MikanResponseFuture::MikanResponseFuture(MikanAPIResult result)
	: m_impl(new MikanResponseFutureImpl())
{
	// Immediately set the result on the future
	MikanResponsePromise promise;
	m_impl->future = promise.get_future();
	promise.set_value(makeSimpleMikanResponse(result));
}

MikanResponseFuture::MikanResponseFuture(
	MikanRequestManager* owner, 
	MikanRequestID requestId, 
	MikanResponsePromise& promise)
	: m_impl(new MikanResponseFutureImpl())
{
	m_impl->ownerRequestManager= owner;
	m_impl->requestId= requestId;
	m_impl->future= promise.get_future();
}

MikanResponseFuture::MikanResponseFuture(MikanResponseFuture&& other) noexcept
	: m_impl(new MikanResponseFutureImpl())
{
	m_impl->ownerRequestManager = other.m_impl->ownerRequestManager;
	m_impl->requestId = other.m_impl->requestId;
	m_impl->future = std::move(other.m_impl->future);
}

MikanResponseFuture::~MikanResponseFuture()
{
	if (m_impl != nullptr)
	{
		delete m_impl;
	}
}

bool MikanResponseFuture::isCompleted() const
{
	if (isValid())
	{
		// This is apparently the only way to check if a future is ready in a "non-blocking" way
		// https://stackoverflow.com/questions/10890242/get-the-status-of-a-stdfuture
		return m_impl->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}
	else
	{
		return false;
	}
}


bool MikanResponseFuture::tryFetchResponse(MikanResponsePtr& outResponse)
{
	if (isCompleted())
	{
		outResponse = m_impl->future.get();
		return true;
	}

	return false;
}

MikanResponsePtr MikanResponseFuture::fetchResponse(uint32_t timeoutMilliseconds)
{
	if (isValid())
	{
		if (timeoutMilliseconds > 0)
		{
			// Wait for a fixed timeout
			if (m_impl->future.wait_for(std::chrono::milliseconds(timeoutMilliseconds)) == std::future_status::ready)
			{
				return m_impl->future.get();
			}
			else
			{
				// Timeout reached, cancel the request
				if (m_impl->ownerRequestManager != nullptr &&
					m_impl->requestId != INVALID_MIKAN_ID)
				{
					m_impl->ownerRequestManager->cancelRequest(m_impl->requestId);
				}

				// Return a timeout response instead
				return makeSimpleMikanResponse(MikanAPIResult::Timeout);
			}
		}
		else
		{
			return m_impl->future.get();
		}
	}
	else
	{
		return makeSimpleMikanResponse(MikanAPIResult::Uninitialized);
	}
}

void MikanResponseFuture::awaitResponse(uint32_t timeoutMilliseconds)
{
	// Drop the response on the floor
	fetchResponse(timeoutMilliseconds);
}

MikanResponsePtr MikanResponseFuture::makeSimpleMikanResponse(MikanAPIResult result)
{
	auto response = std::make_shared<MikanResponse>();
	response->requestId = INVALID_MIKAN_ID;
	response->resultCode = result;

	return response;
}
