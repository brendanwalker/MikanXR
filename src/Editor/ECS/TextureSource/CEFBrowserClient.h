#pragma once

#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_load_handler.h"
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
class CEFBrowserClient : public CefClient,
						 public CefRenderHandler,
						 public CefLifeSpanHandler,
						 public CefDisplayHandler,
						 public CefLoadHandler
{
public:
	explicit CEFBrowserClient(std::weak_ptr<class CEFTextureSourceComponent> owner);

	// -- CefClient ----
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

	// -- CefRenderHandler ----
	virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
						 const void* buffer, int width, int height) override;
	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

	// -- CefLifeSpanHandler ----
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int popup_id,
							   const CefString& target_url, const CefString& target_frame_name,
							   WindowOpenDisposition target_disposition, bool user_gesture,
							   const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
							   CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
							   CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access) override;

	// -- CefDisplayHandler ----
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
								 const CefString& url) override;
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

	// -- CefLoadHandler ----
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
									  bool canGoForward) override;

private:
	std::weak_ptr<class CEFTextureSourceComponent> m_owner;

	IMPLEMENT_REFCOUNTING(CEFBrowserClient);
};
