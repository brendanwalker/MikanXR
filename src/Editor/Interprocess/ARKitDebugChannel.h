#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/// Debug side channel to the MikanARStreamer iPhone app.
///
/// The phone is the TCP client and connects out to this listener, because the
/// phone already knows the editor's host from its streaming config while the
/// editor cannot learn the phone's address: video arrives through a GStreamer
/// udpsrc, which does not surface the peer.
///
/// Two things travel over it. The phone pushes diagnostic lines, which are
/// re-emitted through the editor's own logger so `log tail`, the Log panel, and
/// MikanXR.log pick them up with no separate storage. In the other direction a
/// command line goes to the phone and its reply comes back, exposed as
/// `arkit send`. The command text is opaque here on purpose: the phone owns its
/// own debug vocabulary and can grow it without an editor rebuild.
///
/// Unlike the automation server this binds every interface, so it is off unless
/// asked for (AppSettingsConfig::getARKitDebugChannelEnabled, or the
/// -arkitDebugChannel command line flag).
class ARKitDebugChannel
{
public:
	/// Wire protocol version, exchanged in the handshake. Bump when the line
	/// grammar below changes in a way an older peer would misread.
	static const int k_protocolVersion= 1;

	/// How long `arkit send` waits for the phone before answering an error.
	/// Without this a silent phone leaves the automation client waiting and the
	/// deferred reply state wedged.
	static const int k_commandTimeoutSeconds= 5;

	ARKitDebugChannel();
	~ARKitDebugChannel();

	// Non-copyable
	ARKitDebugChannel(const ARKitDebugChannel&)= delete;
	ARKitDebugChannel& operator=(const ARKitDebugChannel&)= delete;

	/// Open the listener. A failed bind is logged and tolerated, matching the
	/// automation server.
	/// @returns false if the listener socket could not open
	bool startup(uint16_t port);

	/// Service socket I/O and age any in-flight command's timeout.
	/// Called once per frame on the main thread.
	void poll(float deltaSeconds);

	void shutdown();

	/// Register the `arkit` namespace, mirroring
	/// TransactionHistory::registerAutomationCommands.
	void registerAutomationCommands(class AutomationServer* automationServer);

	bool isEnabled() const { return m_socket != nullptr; }
	bool isPhoneConnected() const;

private:
	// ---- Line routing ----
	void onLineReceived(const std::string& line);
	void onClientDisconnected();

	/// First line from a new peer: `hello <protocolVersion> <deviceName...>`.
	void handleHelloLine(const std::string& line);
	/// `log <level> <text...>`, re-emitted through the editor's logger.
	void handleLogLine(const std::string& body);
	/// `reply <n>`, followed by exactly n content lines.
	void handleReplyHeaderLine(const std::string& body);

	/// Answer an in-flight `arkit send` and clear the pending state.
	void completePendingCommand(const std::vector<std::string>& contentLines, bool bIsError);
	void failPendingCommand(const std::string& reason);

	// ---- Automation surface ----
	bool handleARKitCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
							std::string& outError);

	std::unique_ptr<class AutomationSocket> m_socket;
	class AutomationServer* m_automationServer= nullptr;
	uint16_t m_port= 0;

	// Peer identity, valid only once the handshake completes
	bool m_bHandshakeComplete= false;
	int m_peerProtocolVersion= 0;
	std::string m_peerDeviceName;

	// In-flight `arkit send`. Only one at a time, matching the automation
	// server's own one-client-one-outstanding-command model.
	bool m_bCommandPending= false;
	float m_commandTimeoutRemaining= 0.f;

	// Reply accumulation. While m_replyLinesRemaining is non-zero every arriving
	// line is reply content, checked before keyword dispatch so a reply line
	// that happens to begin with "log" is not mistaken for a diagnostic.
	int m_replyLinesRemaining= 0;
	std::vector<std::string> m_replyLines;
};
