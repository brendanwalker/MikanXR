#pragma once

#include "MikanAPI.h"

class MikanScriptAPI : public IMikanScriptAPI
{
public:
	MikanScriptAPI() = default;
	MikanScriptAPI(class MikanRequestManager* requestManager);

	inline static const std::string k_sendScriptMessage = "invokeScriptMessageHandler";
	virtual MikanResponseFuture sendScriptMessage(const std::string& mesg) override;

private:
	class MikanRequestManager* m_requestManager = nullptr;
};