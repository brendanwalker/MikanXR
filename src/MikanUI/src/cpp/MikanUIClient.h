#pragma once

#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_request_handler.h"
#include "include/wrapper/cef_message_router.h"

#include <list>

// MikanUIClient handles browser-level callbacks
class MikanUIClient : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefRequestHandler
{
public:
    MikanUIClient();
    ~MikanUIClient();

    // CefClient methods
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefProcessId source_process,
                                         CefRefPtr<CefProcessMessage> message) override;

    // CefLifeSpanHandler methods
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // Request that all existing browser windows close
    void CloseAllBrowsers(bool force_close);

    bool IsClosing() const { return m_isClosing; }

    void SetParentWindow(HWND hwnd) { m_parentWindow = hwnd; }

    // CefRequestHandler methods
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

private:
    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList m_browserList;

    bool m_isClosing;
    HWND m_parentWindow;

    // Message router for handling JavaScript queries
    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;

    IMPLEMENT_REFCOUNTING(MikanUIClient);
};
