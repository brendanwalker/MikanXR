#include "MikanStencilAPI.h"
#include "MikanRequestManager.h"
#include "MikanStencilTypes_json.h"
#include "MikanStencilTypes_binary.h"

MikanStencilAPI::MikanStencilAPI(class MikanRequestManager* requestManager)
	: m_requestManager(requestManager)
{
	m_requestManager->addTextResponseFactory<MikanStencilList>();
	m_requestManager->addTextResponseFactory<MikanStencilQuadInfo>();
	m_requestManager->addTextResponseFactory<MikanStencilBoxInfo>();
	m_requestManager->addTextResponseFactory<MikanStencilModelInfo>();
	m_requestManager->addBinaryResponseFactory<MikanStencilModelRenderGeometry>();
}

MikanResponseFuture MikanStencilAPI::getQuadStencilList() // returns MikanStencilList
{
	return m_requestManager->sendRequest(k_getQuadStencilList);
}

MikanResponseFuture MikanStencilAPI::getQuadStencil(MikanStencilID stencilId) // returns MikanStencilQuad
{
	return m_requestManager->sendRequestWithPayload<int>(k_getQuadStencil, stencilId);
}

MikanResponseFuture MikanStencilAPI::getBoxStencilList() // returns MikanStencilList
{
	return m_requestManager->sendRequest(k_getBoxStencilList);
}

MikanResponseFuture MikanStencilAPI::getBoxStencil(MikanStencilID stencilId) // returns MikanStencilBox
{
	return m_requestManager->sendRequestWithPayload<int>(k_getBoxStencil, stencilId);
}

MikanResponseFuture MikanStencilAPI::getModelStencilList() // returns MikanStencilList
{
	return m_requestManager->sendRequest(k_getModelStencilList);
}

MikanResponseFuture MikanStencilAPI::getModelStencil(MikanStencilID stencilId) // returns MikanStencilModel
{
	return m_requestManager->sendRequestWithPayload<int>(k_getModelStencil, stencilId);
}

MikanResponseFuture MikanStencilAPI::getModelStencilRenderGeometry(MikanStencilID stencilId) // returns MikanStencilModel
{
	return m_requestManager->sendRequestWithPayload<int>(k_getModelStencilRenderGeometry, stencilId);
}