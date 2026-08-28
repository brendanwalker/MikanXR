#include "AutomationProtocolTests.h"
#include "AutomationProtocol.h"
#include "unit_test.h"

#include <cassert>
#include <string>
#include <vector>

namespace
{
// Tokenize and expect success with the given tokens.
static bool tokenizesTo(const std::string& line, const std::vector<std::string>& expected)
{
	std::vector<std::string> tokens;
	std::string error;
	if (!AutomationProtocol::tokenizeCommandLine(line, tokens, error))
	{
		fprintf(stdout, "    tokenizesTo FAILED: '%s' errored: %s\n", line.c_str(), error.c_str());
		return false;
	}

	if (tokens != expected)
	{
		fprintf(stdout, "    tokenizesTo FAILED: '%s' produced %zu tokens, expected %zu\n", line.c_str(), tokens.size(),
				expected.size());
		return false;
	}

	return true;
}
} // namespace

bool automation_protocol_test_tokenize_plain_words()
{
	UNIT_TEST_BEGIN("plain words split on whitespace runs")

	success&= tokenizesTo("app info", {"app", "info"});
	success&= tokenizesTo("  property   get\tx ", {"property", "get", "x"});
	success&= tokenizesTo("", {});
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_tokenize_quoted_spans()
{
	UNIT_TEST_BEGIN("quoted spans keep spaces and empty tokens")

	success&=
		tokenizesTo("app open \"C:\\Projects\\My Project.mikan\"", {"app", "open", "C:\\Projects\\My Project.mikan"});
	success&= tokenizesTo("set name \"\"", {"set", "name", ""});
	success&= tokenizesTo("a\"b c\"d", {"ab cd"});
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_tokenize_escapes()
{
	UNIT_TEST_BEGIN("escapes inside quotes yield literal quote and backslash")

	success&= tokenizesTo("say \"a \\\"quoted\\\" word\"", {"say", "a \"quoted\" word"});
	success&= tokenizesTo("path \"a\\\\b\"", {"path", "a\\b"});
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_tokenize_unterminated_quote_errors()
{
	UNIT_TEST_BEGIN("unterminated quote reports a parse error")

	std::vector<std::string> tokens;
	std::string error;
	success&= !AutomationProtocol::tokenizeCommandLine("app open \"C:\\unterminated", tokens, error);
	success&= !error.empty();
	success&= tokens.empty();
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_frame_reply_counts_lines()
{
	UNIT_TEST_BEGIN("replies frame as count line plus content lines")

	success&= (AutomationProtocol::frameReply({}) == "0\r\n");
	success&= (AutomationProtocol::frameReply({"value"}) == "1\r\nvalue\r\n");
	success&= (AutomationProtocol::frameReply({"a", "b", "c"}) == "3\r\na\r\nb\r\nc\r\n");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_frame_reply_splits_embedded_newlines()
{
	UNIT_TEST_BEGIN("entries with embedded newlines split so the count stays accurate")

	success&= (AutomationProtocol::frameReply({"line1\nline2"}) == "2\r\nline1\r\nline2\r\n");
	success&= (AutomationProtocol::frameReply({"a\r\nb", "c"}) == "3\r\na\r\nb\r\nc\r\n");
	success&= (AutomationProtocol::frameReply({"trailing\n"}) == "2\r\ntrailing\r\n\r\n");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool automation_protocol_test_remainder_after_tokens()
{
	UNIT_TEST_BEGIN("remainder after tokens returns the raw untokenized tail")

	using AutomationProtocol::remainderAfterTokens;
	success&= (remainderAfterTokens("script eval Sys 12 print(\"a b\")", 4) == "print(\"a b\")");
	success&= (remainderAfterTokens("a  b   c d", 2) == "c d");
	success&= (remainderAfterTokens("\"one token\" tail", 1) == "tail");
	success&= (remainderAfterTokens("a b", 2) == "");
	success&= (remainderAfterTokens("a b", 5) == "");
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool run_automation_protocol_tests()
{
	UNIT_TEST_MODULE_BEGIN("automation_protocol")
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_tokenize_plain_words);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_tokenize_quoted_spans);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_tokenize_escapes);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_tokenize_unterminated_quote_errors);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_frame_reply_counts_lines);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_frame_reply_splits_embedded_newlines);
	UNIT_TEST_MODULE_CALL_TEST(automation_protocol_test_remainder_after_tokens);
	UNIT_TEST_MODULE_END()
}
