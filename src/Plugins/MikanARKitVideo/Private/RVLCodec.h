#pragma once

// -- includes -----
#include <cstdint>
#include <vector>

// Decoder for the RVL (Run-length + Variable-length delta) lossless 16-bit depth
// compression scheme used on the depth channel (see ARKitWireProtocol.h). Mikan only
// ever decodes - the iPhone app is the only encoder - so no encode function is
// exposed here.
//
// Bitstream layout (must match the iPhone-side encoder bit-for-bit):
//   The compressed payload is a sequence of 4-bit nibbles, packed two per byte with
//   the high nibble first. Nibbles form variable-length-encoded (VLE) unsigned
//   integers: each nibble carries 3 payload bits in its low 3 bits, least-significant
//   nibble first, with bit 3 acting as a continuation flag (1 = more nibbles follow).
//   The pixel stream alternates, until expectedPixelCount samples have been produced:
//     zeroRunLength  = VLE                (count of consecutive zero/invalid pixels)
//     nonzeroRunLength = VLE              (count of consecutive nonzero pixels)
//     nonzeroRunLength * { delta = VLE }  (zigzag-encoded delta from the previous
//                                          nonzero pixel value; resets to a delta
//                                          from 0 at the start of every nonzero
//                                          run - NOT carried across zero-run gaps)
//   If the total nibble count is odd, the final byte's low nibble is unused padding.
//   An encoder may omit the final nonzeroRunLength=0 marker when a zeroRun consumes
//   the array exactly to its end (the real iPhone encoder does this) - decoders must
//   not assume that marker is always present once expectedPixelCount is reached.
//
// This is a byte-oriented variant of the classic Wilson "Fast Lossless Depth Image
// Compression" RVL algorithm - deliberately specified as a flat nibble/byte stream
// rather than the original reference's 32-bit int word packing, since the latter is
// implicitly host-endianness-dependent once serialized to bytes (the same class of
// bug this wire protocol otherwise avoids - see ARKitWireProtocol.h).

// Decodes an RVL-compressed depth plane into `expectedPixelCount` uint16 depth
// samples (millimeters, 0=invalid). Returns an empty vector on any malformed,
// truncated, or otherwise invalid input rather than throwing - never reads past
// data[0, length).
std::vector<uint16_t> rvlDecode(const uint8_t* data, size_t length, int expectedPixelCount);

// Decodes a run-length-encoded confidence plane - a sequence of (value, runLength)
// byte pairs, runLength in [1, 255] - into `expectedPixelCount` uint8 confidence
// samples (0/1/2). Returns an empty vector on any malformed or truncated input.
std::vector<uint8_t> packConfidenceRLEDecode(const uint8_t* data, size_t length, int expectedPixelCount);
