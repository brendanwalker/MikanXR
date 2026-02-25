#pragma once

#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"

#include <list>

// MikanUIClient handles browser-level callbacks
class MikanUIClient : public CefClient,
                      public CefLifeSpanHandler
{
public:
    MikanUIClient();
    ~MikanUIClient();

    // CefClient methods
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    // CefLifeSpanHandler methods
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // Request that all existing browser windows close
    void CloseAllBrowsers(bool force_close);

    bool IsClosing() const { return m_isClosing; }

private:
    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList m_browserList;

    bool m_isClosing;

    IMPLEMENT_REFCOUNTING(MikanUIClient);
};
