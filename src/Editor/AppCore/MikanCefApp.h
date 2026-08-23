#pragma once

#include "include/cef_app.h"

// CefApp subclass that injects command-line switches into every CEF process
// (both the main browser process and all renderer/GPU/utility subprocesses).
// Because Mikan.exe acts as its own CEF subprocess handler, this instance
// must be passed to both CefExecuteProcess() and CefInitialize().
class MikanCefApp : public CefApp
{
public:
	// CefApp interface
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type,
											   CefRefPtr<CefCommandLine> command_line) override;

private:
	IMPLEMENT_REFCOUNTING(MikanCefApp);
};
