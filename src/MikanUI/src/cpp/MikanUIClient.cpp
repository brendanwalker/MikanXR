#include "MikanUIClient.h"

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include "tinyfiledialogs.h"
#include "PathUtils.h"

#include <windows.h>
#include <filesystem>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Message handler for processing JavaScript queries
class MessageHandler : public CefMessageRouterBrowserSide::Handler
{
public:
    MessageHandler() {}

    // Called due to cefQuery execution in the JavaScript
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        int64_t query_id,
                        const CefString& request,
                        bool persistent,
                        CefRefPtr<Callback> callback) override
    {
        CEF_REQUIRE_UI_THREAD();

        try
        {
            // Parse the JSON request
            std::string requestStr = request.ToString();
            json requestJson = json::parse(requestStr);

            std::string type = requestJson.value("type", "");

            if (type == "open_file_dialog")
            {
                handleOpenFileDialog(requestJson, callback);
                return true;
            }
            else if (type == "save_file_dialog")
            {
                handleSaveFileDialog(requestJson, callback);
                return true;
            }
        }
        catch (const std::exception& e)
        {
            json errorResponse;
            errorResponse["success"] = false;
            errorResponse["error"] = std::string("Error processing request: ") + e.what();
            callback->Success(errorResponse.dump());
            return true;
        }

        // Request not handled
        return false;
    }

private:
    void handleOpenFileDialog(const json& request, CefRefPtr<Callback> callback)
    {
        std::string title = request.value("title", "Open File");
        std::string defaultPath = request.value("defaultPath", "");
        std::string filter = request.value("filter", "*.*");
        std::string filterDescription = request.value("filterDescription", "All Files");

        // Convert default path
        if (defaultPath.empty())
        {
            defaultPath = (PathUtils::getHomeDirectory() / "").string();
        }

        // Set up filter
        const char* filterPatterns[1] = { filter.c_str() };

        // Show the file dialog
        const char* filePath = tinyfd_openFileDialog(
            title.c_str(),
            defaultPath.c_str(),
            1,
            filterPatterns,
            filterDescription.c_str(),
            0  // single select
        );

        // Build response
        json response;
        if (filePath != nullptr && filePath[0] != '\0')
        {
            response["success"] = true;
            response["filePath"] = filePath;
        }
        else
        {
            response["success"] = false;
            response["canceled"] = true;
        }

        callback->Success(response.dump());
    }

    void handleSaveFileDialog(const json& request, CefRefPtr<Callback> callback)
    {
        std::string title = request.value("title", "Save File");
        std::string defaultPath = request.value("defaultPath", "");
        std::string filter = request.value("filter", "*.*");
        std::string filterDescription = request.value("filterDescription", "All Files");

        // Convert default path
        if (defaultPath.empty())
        {
            defaultPath = (PathUtils::getHomeDirectory() / "").string();
        }

        // Set up filter
        const char* filterPatterns[1] = { filter.c_str() };

        // Show the file dialog
        const char* filePath = tinyfd_saveFileDialog(
            title.c_str(),
            defaultPath.c_str(),
            1,
            filterPatterns,
            filterDescription.c_str()
        );

        // Build response
        json response;
        if (filePath != nullptr && filePath[0] != '\0')
        {
            response["success"] = true;
            response["filePath"] = filePath;
        }
        else
        {
            response["success"] = false;
            response["canceled"] = true;
        }

        callback->Success(response.dump());
    }
};

MikanUIClient::MikanUIClient()
    : m_isClosing(false)
    , m_parentWindow(NULL)
{
    // Create the message router
    CefMessageRouterConfig config;
    m_messageRouter = CefMessageRouterBrowserSide::Create(config);

    // Add our message handler
    m_messageRouter->AddHandler(new MessageHandler(), false);
}

MikanUIClient::~MikanUIClient()
{
}

void MikanUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers
    m_browserList.push_back(browser);

    // Register message router
    m_messageRouter->OnBeforeBrowse(browser, browser->GetMainFrame());
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

    // Unregister message router
    m_messageRouter->OnBeforeClose(browser);

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
    CEF_REQUIRE_UI_THREAD();
    return m_messageRouter->OnProcessMessageReceived(browser, frame, source_process, message);
}