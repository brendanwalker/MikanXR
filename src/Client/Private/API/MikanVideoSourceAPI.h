#pragma once

#include "MikanExport.h"
#include "MikanTypeFwd.h"
#include "MikanAPI.h"

class MikanVideoSourceAPI : public IMikanVideoSourceAPI
{
public:
	MikanVideoSourceAPI() = default;
	MikanVideoSourceAPI(class MikanRequestManager* requestManager);

	inline static const std::string k_getVideoSourceIntrinsics = "getVideoSourceIntrinsics";
	MikanResponseFuture getVideoSourceIntrinsics(); // returns MikanVideoSourceIntrinsics

	inline static const std::string k_getVideoSourceMode = "getVideoSourceMode";
	MikanResponseFuture getVideoSourceMode(); // returns MikanVideoSourceMode

	inline static const std::string k_getVideoSourceAttachment = "getVideoSourceAttachment";
	MikanResponseFuture getVideoSourceAttachment(); // returns MikanVideoSourceAttachmentInfo

private:
	MikanRequestManager* m_requestManager = nullptr;
};
