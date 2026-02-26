#pragma once

#include "include/cef_app.h"

// MikanUIApp implements CefApp and handles process-level callbacks
class MikanUIApp : public CefApp, public CefBrowserProcessHandler
{
public:
    MikanUIApp();

    // CefApp methods
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

    // CefBrowserProcessHandler methods
    virtual void OnContextInitialized() override;

private:
    IMPLEMENT_REFCOUNTING(MikanUIApp);
};
