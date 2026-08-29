#pragma once

#include <string>

/// Pure text-protocol helpers for the ARKit debug channel.
/// No editor dependencies, so the parsing is unit-testable headlessly, the same
/// split AutomationProtocol makes for the automation command server.
///
/// The grammar, all newline terminated:
///   phone -> editor   hello <protocolVersion> <deviceName...>   (first line only)
///   phone -> editor   log <level> <text...>
///   phone -> editor   reply <n>                                 (then n raw lines)
///   editor -> phone   ok <protocolVersion>
///   editor -> phone   cmd <text...>
namespace ARKitDebugProtocol
{
/// What kind of line arrived from the phone after the handshake.
enum class eLineKind
{
	unknown,
	log,
	reply
};

/// Split "<word> <rest...>" on the first whitespace run. The remainder keeps
/// its own spacing and quoting, since relayed text is passed through verbatim.
void splitFirstWord(const std::string& line, std::string& outWord, std::string& outRest);

/// Parse the handshake line "hello <protocolVersion> <deviceName...>".
/// The device name is free text and may be empty.
/// @returns false when the line is not a hello or carries no numeric version
bool parseHello(const std::string& line, int& outVersion, std::string& outDeviceName);

/// Classify a post-handshake line, returning the text after the keyword.
eLineKind classifyLine(const std::string& line, std::string& outBody);

/// Parse a log body "<level> <text...>", where level is one of the logger's own
/// names (trace, debug, info, warning, error, fatal) and outLevel is the
/// matching LogSeverityLevel value as an int, the same encoding
/// AutomationLogBuffer uses.
/// An unrecognized level is not an error: it falls back to info with the whole
/// body as the text, so a vocabulary mismatch loses formatting rather than
/// dropping a diagnostic.
void parseLogBody(const std::string& body, int& outLevel, std::string& outText);

/// Parse the line count from a "reply <n>" body.
/// @returns false when the count is not a number, is negative, or exceeds
///          maxLines, which bounds what a broken peer can make us accumulate
bool parseReplyCount(const std::string& body, int maxLines, int& outCount);
} // namespace ARKitDebugProtocol
