#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

/// The automation command server: a loopback TCP text channel used by test
/// scripts and AI tooling to drive and inspect the running editor.
/// Commands are newline-terminated "<namespace> [verb] [args...]" lines.
/// Every command is answered with one framed reply: a line holding the
/// content line count, then exactly that many lines.
///
/// This channel is deliberately separate from the websocket client API,
/// which serves shipped client applications.
class AutomationServer
{
public:
	/// Handles one command. args holds the tokens after the namespace name
	/// (args[0] is the verb where the namespace has verbs).
	/// On success, fill outLines (empty is a valid reply).
	/// On failure, return false with outError set; the server frames it as
	/// "<namespace> <verb>: <error>".
	using CommandHandler= std::function<bool(const std::vector<std::string>& args, std::vector<std::string>& outLines,
											 std::string& outError)>;

	AutomationServer();
	~AutomationServer();

	// Non-copyable
	AutomationServer(const AutomationServer&)= delete;
	AutomationServer& operator=(const AutomationServer&)= delete;

	/// Open the loopback listener and register the built-in command namespaces.
	/// @returns false if the listener socket could not open
	bool startup(class MainWindow* mainWindow, uint16_t port);

	/// Service socket I/O and dispatch any received commands.
	/// Called once per frame on the main thread, so handlers may call editor
	/// code directly and reply synchronously.
	void poll();

	void shutdown();

	/// Capture the window back buffer for a pending screenshot window command
	/// and send its deferred reply. Called from the main window's render,
	/// after rendering completes and before the frame presents.
	void servicePendingWindowCapture(int windowWidth, int windowHeight);

	/// Register a command namespace. Future features (transaction history,
	/// metrics, ...) add their surface here.
	void registerCommandNamespace(const std::string& namespaceName, const std::vector<std::string>& helpLines,
								  CommandHandler handler);

	/// Suppress the reply to the command currently dispatching, so a handler
	/// that cannot answer yet can answer later through sendDeferredReply.
	/// Nothing bounds the wait to this frame, so a handler may park across a
	/// network round trip. A handler that defers owns answering: it must arm
	/// its own timeout, or the waiting client never hears back.
	void deferReply() { m_bReplyDeferred= true; }

	/// Answer a command whose handler called deferReply. outLines is framed as
	/// a normal reply; pass bIsError to frame it the way a handler returning
	/// false would be framed.
	void sendDeferredReply(const std::vector<std::string>& contentLines, bool bIsError= false);

	/// The raw, untokenized line of the command currently dispatching, for
	/// handlers whose final argument is free text. Pair with
	/// AutomationProtocol::remainderAfterTokens.
	const std::string& getCurrentCommandLine() const { return m_currentCommandLine; }

private:
	void handleCommandLine(const std::string& line);
	void sendReply(const std::vector<std::string>& contentLines);
	void sendErrorReply(const std::string& errorLine);

	// Built-in command namespaces
	void registerCoreNamespaces();
	bool handleAppCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
						  std::string& outError);
	bool handleStageCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							std::string& outError);
	bool handleHelpCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
						   std::string& outError);
	bool handleSystemCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							 std::string& outError);
	bool handleComponentCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
								std::string& outError);
	bool handlePropertyCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							   std::string& outError);
	bool handleFunctionCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							   std::string& outError);
	bool handleScreenshotCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
								 std::string& outError);
	bool handleScriptCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							 std::string& outError);
	bool handleLogCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
						  std::string& outError);
	bool handleNodeGraphCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
								std::string& outError);

	struct CommandProvider
	{
		std::vector<std::string> helpLines;
		CommandHandler handler;
	};

	class MainWindow* m_mainWindow= nullptr;
	std::unique_ptr<class AutomationSocket> m_socket;
	std::map<std::string, CommandProvider> m_commandProviders;

	// The raw line of the command currently dispatching, for handlers whose
	// final argument is free text that must not go through tokenization
	std::string m_currentCommandLine;

	// A handler that parked work for later in the frame sets this to suppress
	// the immediate reply; the parked work sends the reply when it completes
	bool m_bReplyDeferred= false;

	// Pending screenshot window capture, serviced at end of frame render
	bool m_bWindowCapturePending= false;
	std::string m_windowCapturePath;
};
