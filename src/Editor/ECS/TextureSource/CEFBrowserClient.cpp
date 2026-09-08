#include "CEFBrowserClient.h"
#include "CEFTextureSourceComponent.h"
#include "include/cef_browser.h"

// -- CEFBrowserClient ------
CEFBrowserClient::CEFBrowserClient(std::weak_ptr<CEFTextureSourceComponent> owner)
	: m_owner(std::move(owner))
{
}

void CEFBrowserClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefGetViewRect(rect);
	}
	else
	{
		rect= CefRect(0, 0, 1280, 720);
	}
}

void CEFBrowserClient::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
							   const void* buffer, int width, int height)
{
	auto comp= m_owner.lock();
	if (!comp)
		return;

	if (type == PET_VIEW)
	{
		comp->onCefPaint(buffer, width, height);
	}
	else if (type == PET_POPUP)
	{
		// Dropdowns, autofill lists and other widget popups paint into their own surface.
		// The component composites them over the view before uploading the texture.
		comp->onCefPopupPaint(buffer, width, height);
	}
}

void CEFBrowserClient::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefPopupShow(show);
	}
}

void CEFBrowserClient::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefPopupSize(rect.x, rect.y, rect.width, rect.height);
	}
}

void CEFBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefBrowserCreated(browser);
	}
}

void CEFBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefBrowserClosed();
	}
}

bool CEFBrowserClient::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int popup_id,
									 const CefString& target_url, const CefString& target_frame_name,
									 WindowOpenDisposition target_disposition, bool user_gesture,
									 const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
									 CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
									 CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access)
{
	// A new popup browser would be windowed and would never call OnPaint, so it could not reach the
	// scene texture. Navigate the existing windowless browser instead and cancel the popup.
	if (frame && !target_url.empty())
	{
		frame->LoadURL(target_url);
	}

	return true;
}

void CEFBrowserClient::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
	// Sub-frame navigation does not change what the address bar should show
	if (!frame || !frame->IsMain())
		return;

	if (auto comp= m_owner.lock())
	{
		comp->onCefAddressChange(url.ToString());
	}
}

void CEFBrowserClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefTitleChange(title.ToString());
	}
}

void CEFBrowserClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
											bool canGoForward)
{
	if (auto comp= m_owner.lock())
	{
		comp->onCefLoadingStateChange(isLoading, canGoBack, canGoForward);
	}
}
