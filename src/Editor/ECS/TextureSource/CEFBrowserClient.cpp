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
	if (type != PET_VIEW)
		return;

	if (auto comp= m_owner.lock())
	{
		comp->onCefPaint(buffer, width, height);
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