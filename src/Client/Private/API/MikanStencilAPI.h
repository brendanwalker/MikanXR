#pragma once

#include "MikanAPI.h"

class MikanStencilAPI : public IMikanStencilAPI
{
public:
	MikanStencilAPI() = default;
	MikanStencilAPI(class MikanRequestManager* requestManager);

	inline static const std::string k_getStencilList = "getStencilList";
	virtual MikanResponseFuture getStencilList() override; // returns MikanStencilList

	inline static const std::string k_getQuadStencil = "getQuadStencil";
	virtual MikanResponseFuture getQuadStencil(MikanStencilID stencilId) override; // returns MikanStencilQuad

	inline static const std::string k_getBoxStencil = "getBoxStencil";
	virtual MikanResponseFuture getBoxStencil(MikanStencilID stencilId) override; // returns MikanStencilBox

	inline static const std::string k_getModelStencil = "getModelStencil";
	virtual MikanResponseFuture getModelStencil(MikanStencilID stencilId) override; // returns MikanStencilModel

private:
	class MikanRequestManager* m_requestManager = nullptr;
};
