#include "MikanUIApp.h"
#include "MikanUIClient.h"

#include "include/cef_browser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#include <windows.h>
#include <shlwapi.h>

MikanUIApp::MikanUIApp()
{
}

void MikanUIApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    CefRefPtr<MikanUIClient> client = new MikanUIClient();

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    // Get the executable directory
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    PathRemoveFileSpecA(exePath);

    std::string url = std::string("file:///") + exePath + "/resources/index.html";

    // Information used when creating the native window.
    CefWindowInfo window_info;

#if defined(OS_WIN)
    // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
    window_info.SetAsPopup(NULL, "MikanUI");
#endif

    // Create the browser window.
    CefBrowserHost::CreateBrowser(window_info, client, url, browser_settings, nullptr, nullptr);
}
