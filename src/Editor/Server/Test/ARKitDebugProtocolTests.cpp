#include "ARKitDebugProtocolTests.h"
#include "ARKitDebugProtocol.h"
#include "unit_test.h"

#include <cassert>
#include <string>

namespace
{
const int k_traceLevel= 0;
const int k_infoLevel= 2;
const int k_warningLevel= 3;
const int k_maxReplyLines= 1000;
} // namespace

bool arkit_debug_protocol_test_split_first_word()
{
	UNIT_TEST_BEGIN("splitting the first word leaves the remainder verbatim")

	std::string word;
	std::string rest;

	ARKitDebugProtocol::splitFirstWord("log info hello  world", word, rest);
	success&= (word == "log");
	// The remainder keeps its own internal spacing, since relayed text passes
	// through untouched
	success&= (rest == "info hello  world");

	ARKitDebugProtocol::splitFirstWord("solo", word, rest);
	success&= (word == "solo");
	success&= (rest.empty());

	ARKitDebugProtocol::splitFirstWord("", word, rest);
	success&= (word.empty());
	success&= (rest.empty());

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool arkit_debug_protocol_test_hello_accepts_valid_handshake()
{
	UNIT_TEST_BEGIN("a well-formed hello parses into a version and device name")

	int version= 0;
	std::string deviceName;

	success&= ARKitDebugProtocol::parseHello("hello 1 Test iPhone", version, deviceName);
	success&= (version == 1);
	success&= (deviceName == "Test iPhone");

	// The device name is optional
	version= 0;
	deviceName= "stale";
	success&= ARKitDebugProtocol::parseHello("hello 2", version, deviceName);
	success&= (version == 2);
	success&= (deviceName.empty());

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool arkit_debug_protocol_test_hello_rejects_malformed_handshake()
{
	UNIT_TEST_BEGIN("a malformed hello is rejected rather than defaulted")

	int version= 0;
	std::string deviceName;

	// Wrong keyword: anything before the handshake must be a hello
	success&= !ARKitDebugProtocol::parseHello("log info too early", version, deviceName);
	success&= !ARKitDebugProtocol::parseHello("", version, deviceName);

	// A non-numeric version must not silently read as version 0, which would
	// otherwise be compared against the real version and rejected for the
	// wrong reason
	success&= !ARKitDebugProtocol::parseHello("hello v1 iPhone", version, deviceName);
	success&= !ARKitDebugProtocol::parseHello("hello", version, deviceName);
	success&= !ARKitDebugProtocol::parseHello("hello -1 iPhone", version, deviceName);

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool arkit_debug_protocol_test_classify_line()
{
	UNIT_TEST_BEGIN("post-handshake lines classify by keyword")

	using ARKitDebugProtocol::eLineKind;
	std::string body;

	success&= (ARKitDebugProtocol::classifyLine("log warning dropped a frame", body) == eLineKind::log);
	success&= (body == "warning dropped a frame");

	success&= (ARKitDebugProtocol::classifyLine("reply 3", body) == eLineKind::reply);
	success&= (body == "3");

	success&= (ARKitDebugProtocol::classifyLine("hello 1 iPhone", body) == eLineKind::unknown);
	success&= (ARKitDebugProtocol::classifyLine("", body) == eLineKind::unknown);
	// A near miss is not a log line
	success&= (ARKitDebugProtocol::classifyLine("logging something", body) == eLineKind::unknown);

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool arkit_debug_protocol_test_log_body_levels()
{
	UNIT_TEST_BEGIN("log bodies split into a severity and text")

	int level= -1;
	std::string text;

	ARKitDebugProtocol::parseLogBody("warning encode stalled", level, text);
	success&= (level == k_warningLevel);
	success&= (text == "encode stalled");

	ARKitDebugProtocol::parseLogBody("trace frameSeq=12", level, text);
	success&= (level == k_traceLevel);
	success&= (text == "frameSeq=12");

	// An unrecognized level keeps the whole body as the message rather than
	// eating the first word, so a vocabulary mismatch loses formatting instead
	// of dropping a diagnostic
	ARKitDebugProtocol::parseLogBody("verbose something happened", level, text);
	success&= (level == k_infoLevel);
	success&= (text == "verbose something happened");

	ARKitDebugProtocol::parseLogBody("", level, text);
	success&= (level == k_infoLevel);
	success&= (text.empty());

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool arkit_debug_protocol_test_reply_count_bounds()
{
	UNIT_TEST_BEGIN("reply counts reject non-numeric and out-of-bound values")

	int count= -1;

	success&= ARKitDebugProtocol::parseReplyCount("0", k_maxReplyLines, count);
	success&= (count == 0);

	success&= ARKitDebugProtocol::parseReplyCount("7", k_maxReplyLines, count);
	success&= (count == 7);

	// Non-numeric must not read as 0, which would silently complete a command
	// with an empty reply
	success&= !ARKitDebugProtocol::parseReplyCount("x", k_maxReplyLines, count);
	success&= !ARKitDebugProtocol::parseReplyCount("", k_maxReplyLines, count);

	// A leading minus is not all digits, so negatives are rejected by the same
	// check rather than needing their own
	success&= !ARKitDebugProtocol::parseReplyCount("-1", k_maxReplyLines, count);

	// Bounded so a broken peer cannot make the channel accumulate without limit
	success&= !ARKitDebugProtocol::parseReplyCount("1001", k_maxReplyLines, count);
	success&= !ARKitDebugProtocol::parseReplyCount("99999999999", k_maxReplyLines, count);

	assert(success);

	UNIT_TEST_COMPLETE()
}

bool run_arkit_debug_protocol_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_debug_protocol")
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_split_first_word);
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_hello_accepts_valid_handshake);
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_hello_rejects_malformed_handshake);
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_classify_line);
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_log_body_levels);
	UNIT_TEST_MODULE_CALL_TEST(arkit_debug_protocol_test_reply_count_bounds);
	UNIT_TEST_MODULE_END()
}
