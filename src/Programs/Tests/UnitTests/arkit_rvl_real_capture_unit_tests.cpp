//-- includes -----
#include <assert.h>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "RVLCodec.h"
#include "unit_test.h"

#include <nlohmann/json.hpp>

using json= nlohmann::json;

// Cross-checks the real Mikan-side RVL decoder (RVLCodec.h/.cpp) against real
// captured depth data from docs/rvl_test_vectors.json (ticket A9's original
// goal). Complements arkit_rvl_codec_unit_tests.cpp's synthetic/fuzz coverage and
// arkit_rvl_swift_crosscheck_unit_tests.cpp's hand-transliterated-oracle
// coverage, neither of which exercises bytes actually produced by the real
// on-device Swift encoder - see that file's own header comment on exactly this
// gap. See docs/README.md for the fixture schema and how to add more vectors.
namespace
{
struct RVLTestVector
{
	std::string description;
	int width= 0;
	int height= 0;
	std::vector<uint16_t> input;
	std::string expectedCompressedHex;
};

std::optional<std::vector<uint8_t>> hexToBytes(const std::string& hex)
{
	if (hex.size() % 2 != 0)
		return std::nullopt;

	std::vector<uint8_t> bytes;
	bytes.reserve(hex.size() / 2);
	for (size_t i= 0; i < hex.size(); i+= 2)
	{
		const std::string byteStr= hex.substr(i, 2);
		size_t parsedChars= 0;
		int value= 0;
		try
		{
			value= std::stoi(byteStr, &parsedChars, 16);
		}
		catch (...)
		{
			return std::nullopt;
		}
		if (parsedChars != 2)
			return std::nullopt;

		bytes.push_back(static_cast<uint8_t>(value));
	}

	return bytes;
}

// Returns nullopt if the fixture doesn't exist or fails to parse - callers treat
// that as "not populated yet" (skip), not a hard test failure, since this file is
// hand-populated from a real device capture, not generated at build time.
std::optional<std::vector<RVLTestVector>> loadRealCaptureVectors()
{
#ifndef ARKIT_RVL_TEST_VECTORS_PATH
	return std::nullopt;
#else
	std::ifstream file(ARKIT_RVL_TEST_VECTORS_PATH);
	if (!file.is_open())
		return std::nullopt;

	json root;
	try
	{
		file >> root;
	}
	catch (...)
	{
		return std::nullopt;
	}

	if (!root.is_array())
		return std::nullopt;

	std::vector<RVLTestVector> vectors;
	for (const auto& entry : root)
	{
		RVLTestVector vector;
		vector.description= entry.value("description", std::string());
		vector.width= entry.value("width", 0);
		vector.height= entry.value("height", 0);
		vector.expectedCompressedHex= entry.value("expected_compressed_hex", std::string());

		if (entry.contains("input") && entry["input"].is_array())
		{
			for (const auto& value : entry["input"])
				vector.input.push_back(static_cast<uint16_t>(value.get<int>()));
		}

		vectors.push_back(std::move(vector));
	}

	return vectors;
#endif
}
} // namespace

//-- private functions -----
static bool arkit_rvl_real_capture_test_decode_matches_real_captures()
{
	UNIT_TEST_BEGIN("Mikan decoder matches real on-device captures (docs/rvl_test_vectors.json)")

	const std::optional<std::vector<RVLTestVector>> vectors= loadRealCaptureVectors();
	if (!vectors.has_value() || vectors->empty())
	{
		printf("    [SKIPPED] docs/rvl_test_vectors.json not found or empty - capture real vectors via "
			   "MikanARStreamer's Settings screen (Debug: RVL Test Vector Capture) first, see docs/README.md\n");
		UNIT_TEST_COMPLETE()
	}

	success= true;
	for (const RVLTestVector& vector : *vectors)
	{
		if (static_cast<int>(vector.input.size()) != vector.width * vector.height)
		{
			printf("    [%s] input length (%zu) doesn't match width*height (%d)\n", vector.description.c_str(),
				   vector.input.size(), vector.width * vector.height);
			success= false;
			break;
		}

		const std::optional<std::vector<uint8_t>> compressedBytes= hexToBytes(vector.expectedCompressedHex);
		if (!compressedBytes.has_value())
		{
			printf("    [%s] expected_compressed_hex isn't valid hex\n", vector.description.c_str());
			success= false;
			break;
		}

		const std::vector<uint16_t> decoded=
			rvlDecode(compressedBytes->data(), compressedBytes->size(), static_cast<int>(vector.input.size()));

		if (decoded != vector.input)
		{
			printf("    [%s] decode(realCapturedBytes) != realCapturedInput\n", vector.description.c_str());
			success= false;
			break;
		}
	}
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_rvl_real_capture_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_rvl_real_capture")
	UNIT_TEST_MODULE_CALL_TEST(arkit_rvl_real_capture_test_decode_matches_real_captures);
	UNIT_TEST_MODULE_END()
}
