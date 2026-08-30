#include "ARKitDebugChannel.h"
#include "ARKitDebugProtocol.h"
#include "AutomationProtocol.h"
#include "AutomationServer.h"
#include "AutomationSocket.h"
#include "Logger.h"

#include <cstdlib>
#include <functional>

namespace
{
// A broken peer claiming a huge reply would otherwise accumulate without bound
const int k_maxReplyLines= 1000;
} // namespace

// ---- ARKitDebugChannel -----------------------------------------------------

ARKitDebugChannel::ARKitDebugChannel()= default;

ARKitDebugChannel::~ARKitDebugChannel() { shutdown(); }

bool ARKitDebugChannel::startup(uint16_t port)
{
	m_port= port;

	auto socket= std::make_unique<AutomationSocket>(port, AutomationSocket::eBindScope::anyInterface, "ARKitDebug");
	if (!socket->isListening())
		return false;

	socket->onLineReceived= [this](const std::string& line) { onLineReceived(line); };
	socket->onClientConnected= [this]() { onClientConnected(); };
	socket->onClientDisconnected= [this]() { onClientDisconnected(); };

	m_socket= std::move(socket);
	return true;
}

void ARKitDebugChannel::poll(float deltaSeconds)
{
	if (m_socket == nullptr)
		return;

	m_socket->poll();

	// A peer that connects and never identifies itself holds the single client
	// slot, so drop it and go back to listening
	if (m_socket->isClientConnected() && !m_bHandshakeComplete)
	{
		m_handshakeTimeoutRemaining-= deltaSeconds;
		if (m_handshakeTimeoutRemaining <= 0.f)
		{
			MIKAN_LOG_WARNING("ARKitDebugChannel")
				<< "Dropping a peer that connected but never sent a hello within " << k_handshakeTimeoutSeconds << "s";
			m_socket->disconnectClient("handshake timed out");
		}
	}

	if (m_bCommandPending)
	{
		m_commandTimeoutRemaining-= deltaSeconds;
		if (m_commandTimeoutRemaining <= 0.f)
			failPendingCommand("timed out waiting for the phone");
	}
}

void ARKitDebugChannel::shutdown()
{
	failPendingCommand("the debug channel shut down");
	m_socket.reset();
	m_bHandshakeComplete= false;
	m_peerProtocolVersion= 0;
	m_peerDeviceName.clear();
}

bool ARKitDebugChannel::isPhoneConnected() const
{
	return m_socket != nullptr && m_socket->isClientConnected() && m_bHandshakeComplete;
}

// ---- Line routing ----------------------------------------------------------

void ARKitDebugChannel::onLineReceived(const std::string& line)
{
	// A peer must identify itself before anything else is accepted
	if (!m_bHandshakeComplete)
	{
		handleHelloLine(line);
		return;
	}

	// Reply content is consumed verbatim and ahead of keyword dispatch, so a
	// reply line that happens to start with "log" stays reply content
	if (m_replyLinesRemaining > 0)
	{
		m_replyLines.push_back(line);
		--m_replyLinesRemaining;

		if (m_replyLinesRemaining == 0)
			completePendingCommand(m_replyLines, false);

		return;
	}

	std::string body;
	switch (ARKitDebugProtocol::classifyLine(line, body))
	{
	case ARKitDebugProtocol::eLineKind::log:
		handleLogLine(body);
		break;
	case ARKitDebugProtocol::eLineKind::reply:
		handleReplyHeaderLine(body);
		break;
	default:
		MIKAN_LOG_WARNING("ARKitDebugChannel") << "Ignoring unrecognized line: " << line;
		break;
	}
}

void ARKitDebugChannel::onClientConnected()
{
	m_bHandshakeComplete= false;
	m_handshakeTimeoutRemaining= (float)k_handshakeTimeoutSeconds;
}

void ARKitDebugChannel::onClientDisconnected()
{
	failPendingCommand("the phone disconnected");

	m_bHandshakeComplete= false;
	m_peerProtocolVersion= 0;
	m_peerDeviceName.clear();
	m_replyLinesRemaining= 0;
	m_replyLines.clear();
}

void ARKitDebugChannel::handleHelloLine(const std::string& line)
{
	int peerVersion= 0;
	std::string deviceName;
	if (!ARKitDebugProtocol::parseHello(line, peerVersion, deviceName))
	{
		MIKAN_LOG_WARNING("ARKitDebugChannel") << "Expected a hello line, got: " << line;
		m_socket->disconnectClient("handshake did not start with a valid hello");
		return;
	}

	if (peerVersion != k_protocolVersion)
	{
		MIKAN_LOG_WARNING("ARKitDebugChannel") << "Rejecting a peer speaking protocol version " << peerVersion
											   << ", this build speaks " << k_protocolVersion;
		m_socket->sendText("error unsupported protocol version\n");
		m_socket->disconnectClient("protocol version mismatch");
		return;
	}

	m_peerProtocolVersion= peerVersion;
	m_peerDeviceName= deviceName.empty() ? "unknown" : deviceName;
	m_bHandshakeComplete= true;

	m_socket->sendText("ok " + std::to_string(k_protocolVersion) + "\n");

	MIKAN_LOG_INFO("ARKitDebugChannel") << "Phone connected: " << m_peerDeviceName << " at "
										<< m_socket->getClientAddress() << " (protocol " << m_peerProtocolVersion
										<< ")";
}

void ARKitDebugChannel::handleLogLine(const std::string& body)
{
	int level= 0;
	std::string text;
	ARKitDebugProtocol::parseLogBody(body, level, text);

	const LogSeverityLevel severity= (LogSeverityLevel)level;
	if (!log_can_emit_level(severity))
		return;

	// Emitted in the same shape as the MIKAN_LOG_* macros, so a relayed line
	// sits in the log exactly like an editor line and reads back through
	// `log tail` on the same timeline
	LoggerStream(severity) << log_get_timestamp_prefix() << "iPhone" << " - " << text;
}

void ARKitDebugChannel::handleReplyHeaderLine(const std::string& body)
{
	if (!m_bCommandPending)
	{
		MIKAN_LOG_WARNING("ARKitDebugChannel") << "Ignoring an unsolicited reply from the phone";
		return;
	}

	int lineCount= 0;
	if (!ARKitDebugProtocol::parseReplyCount(body, k_maxReplyLines, lineCount))
	{
		failPendingCommand("the phone sent a malformed reply line count");
		return;
	}

	m_replyLines.clear();
	m_replyLinesRemaining= lineCount;

	if (lineCount == 0)
		completePendingCommand({}, false);
}

void ARKitDebugChannel::completePendingCommand(const std::vector<std::string>& contentLines, bool bIsError)
{
	if (!m_bCommandPending)
		return;

	// Copy before the reset: the success path passes m_replyLines itself, which
	// the clear below would otherwise empty out from under this call
	const std::vector<std::string> reply= contentLines;

	m_bCommandPending= false;
	m_commandTimeoutRemaining= 0.f;
	m_replyLinesRemaining= 0;
	m_replyLines.clear();

	if (m_automationServer != nullptr)
		m_automationServer->sendDeferredReply(reply, bIsError);
}

void ARKitDebugChannel::failPendingCommand(const std::string& reason)
{
	if (!m_bCommandPending)
		return;

	completePendingCommand({"arkit send: " + reason}, true);
}

// ---- Automation surface ----------------------------------------------------

void ARKitDebugChannel::registerAutomationCommands(AutomationServer* automationServer)
{
	using namespace std::placeholders;

	m_automationServer= automationServer;

	automationServer->registerCommandNamespace("arkit", {"arkit status", "arkit send <text...>"},
											   std::bind(&ARKitDebugChannel::handleARKitCommand, this, _1, _2, _3));
}

bool ARKitDebugChannel::handleARKitCommand(const std::vector<std::string>& args, std::vector<std::string>& outLines,
										   std::string& outError)
{
	if (args.empty())
	{
		outError= "usage: arkit status|send";
		return false;
	}

	const std::string& verb= args[0];

	if (verb == "status")
	{
		const bool bConnected= isPhoneConnected();

		outLines.push_back(std::string("enabled ") + (m_socket != nullptr ? "true" : "false"));
		outLines.push_back("port " + std::to_string(m_port));
		outLines.push_back(std::string("connected ") + (bConnected ? "true" : "false"));
		outLines.push_back("address " + (bConnected ? m_socket->getClientAddress() : std::string("none")));
		outLines.push_back("device " + (bConnected ? m_peerDeviceName : std::string("none")));
		outLines.push_back("protocol " + std::to_string(bConnected ? m_peerProtocolVersion : 0));
		return true;
	}

	if (verb == "send")
	{
		if (!isPhoneConnected())
		{
			outError= "no phone connected";
			return false;
		}

		if (m_bCommandPending)
		{
			outError= "a command is already in flight";
			return false;
		}

		if (m_automationServer == nullptr)
		{
			outError= "the automation server is not available";
			return false;
		}

		// Raw rest of the line, so the phone receives the text exactly as typed
		const std::string commandText=
			AutomationProtocol::remainderAfterTokens(m_automationServer->getCurrentCommandLine(), 2);
		if (commandText.empty())
		{
			outError= "usage: arkit send <text...>";
			return false;
		}

		if (!m_socket->sendText("cmd " + commandText + "\n"))
		{
			outError= "failed to send to the phone";
			return false;
		}

		m_bCommandPending= true;
		m_commandTimeoutRemaining= (float)k_commandTimeoutSeconds;
		m_replyLinesRemaining= 0;
		m_replyLines.clear();

		// The phone answers on a later poll, so park this reply
		m_automationServer->deferReply();
		return true;
	}

	outError= "unknown verb '" + verb + "'";
	return false;
}
