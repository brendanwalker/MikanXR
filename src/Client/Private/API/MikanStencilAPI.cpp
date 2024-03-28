#include "MikanStencilAPI.h"
#include "MikanRequestManager.h"
#include "MikanStencilTypes_json.h"

MikanStencilAPI::MikanStencilAPI(class MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
	m_requestManager->addResponseFactory<MikanStencilList>();
	m_requestManager->addResponseFactory<MikanStencilQuad>();
	m_requestManager->addResponseFactory<MikanStencilBox>();
	m_requestManager->addResponseFactory<MikanStencilModel>();
}

MikanResponseFuture MikanStencilAPI::getStencilList() // returns MikanStencilList
{
	return m_requestManager->sendRequest(k_getStencilList);
}

MikanResponseFuture MikanStencilAPI::getQuadStencil(MikanStencilID stencilId) // returns MikanStencilQuad
{
	return m_requestManager->sendRequestWithPayload<int>(k_getQuadStencil, stencilId);
}

MikanResponseFuture MikanStencilAPI::getBoxStencil(MikanStencilID stencilId) // returns MikanStencilBox
{
	return m_requestManager->sendRequestWithPayload<int>(k_getBoxStencil, stencilId);
}

MikanResponseFuture MikanStencilAPI::getModelStencil(MikanStencilID stencilId) // returns MikanStencilModel
{
	return m_requestManager->sendRequestWithPayload<int>(k_getModelStencil, stencilId);
}