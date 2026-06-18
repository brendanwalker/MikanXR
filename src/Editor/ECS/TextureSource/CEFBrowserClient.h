#pragma once

#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_render_handler.h"

#include <mutex>
#include <vector>

// -------------------------------------------------------------------------------------------------
// CEFBrowserClient
//
// Owns all CefClient/CefRenderHandler/CefLifeSpanHandler interfaces so they are managed by CEF's
// own refcounting (IMPLEMENT_REFCOUNTING) instead of by std::shared_ptr. This decouples the two
// lifetime systems: the component is owned by the ECS, the client is owned by CEF. The client
// holds only a weak_ptr back to the component so it can be safely destroyed first.
// -------------------------------------------------------------------------------------------------
class CEFBrowserClient
	: public CefClient
	, public CefRenderHandler
	, public CefLifeSpanHandler
{
public:
	explicit CEFBrowserClient(std::weak_ptr<class CEFTextureSourceComponent> owner);

	// -- CefClient ----
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

	// -- CefRenderHandler ----
	virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
		const RectList& dirtyRects, const void* buffer, int width, int height) override;

	// -- CefLifeSpanHandler ----
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

private:
	std::weak_ptr<class CEFTextureSourceComponent> m_owner;

	IMPLEMENT_REFCOUNTING(CEFBrowserClient);
};