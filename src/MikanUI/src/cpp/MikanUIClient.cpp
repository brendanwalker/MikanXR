#include "MikanUIClient.h"

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include <windows.h>

MikanUIClient::MikanUIClient()
    : m_isClosing(false)
    , m_parentWindow(NULL)
{
}

MikanUIClient::~MikanUIClient()
{
}

void MikanUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers
    m_browserList.push_back(browser);
}

bool MikanUIClient::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void MikanUIClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers
    BrowserList::iterator it = m_browserList.begin();
    for (; it != m_browserList.end(); ++it)
    {
        if ((*it)->IsSame(browser))
        {
            m_browserList.erase(it);
            break;
        }
    }

    if (m_browserList.empty())
    {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void MikanUIClient::CloseAllBrowsers(bool force_close)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&MikanUIClient::CloseAllBrowsers, this, force_close));
        return;
    }

    if (m_browserList.empty())
        return;

    m_isClosing = true;

    BrowserList::const_iterator it = m_browserList.begin();
    for (; it != m_browserList.end(); ++it)
    {
        (*it)->GetHost()->CloseBrowser(force_close);
    }
}

// CefClient methods
bool MikanUIClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
	return true;
}