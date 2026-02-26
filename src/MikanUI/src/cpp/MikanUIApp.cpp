#include "MikanUIApp.h"
#include "MikanUIClient.h"

#include "include/cef_browser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#if defined(OS_WIN)
#include <windows.h>
#include <shlwapi.h>
#endif

#if defined(OS_WIN)
// Window procedure for the parent window
LRESULT CALLBACK MikanUIWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
        {
            // Resize the CEF browser child window when parent is resized
            HWND hBrowser = GetWindow(hwnd, GW_CHILD);
            if (hBrowser)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                SetWindowPos(hBrowser, NULL, 0, 0, rect.right, rect.bottom, SWP_NOZORDER);
            }
            return 0;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
#endif

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
    // Create a window with title bar, minimize, and close buttons
    CefRect rect = {0, 0, 350, 720};  // Initial window size

    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Register window class (Unicode version)
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MikanUIWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"MikanUIWindowClass";

    if (!RegisterClassExW(&wc))
    {
        // Check if already registered
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return;
        }
    }

    // Create a parent window with title bar, minimize, and close buttons (Unicode version)
    HWND parent = CreateWindowExW(
        0,
        L"MikanUIWindowClass",
        L"MikanUI",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,  // Standard window with title bar and controls
        100, 100,
        rect.width,
        rect.height,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!parent)
    {
        return;
    }

    // Store the parent window in the client so it can prevent title changes
    client->SetParentWindow(parent);

    // Set the browser as a child window (no browser chrome - just content)
    window_info.SetAsChild(parent, rect);
#endif

    // Create the browser window.
    CefBrowserHost::CreateBrowser(window_info, client, url, browser_settings, nullptr, nullptr);
}
