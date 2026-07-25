# RVL cross-check test vectors (ticket A9)

`rvl_test_vectors.json` holds real, on-device-captured depth frames used to
cross-check the iPhone-side RVL encoder (`RVLCodec.swift`) against the
Mikan-side decoder (`RVLCodec.cpp`), catching any algorithm mismatch that
synthetic test data wouldn't surface.

This file is duplicated (not symlinked) in both repos:
- `MikanARStreamer/docs/rvl_test_vectors.json` — loaded by
  `MikanARStreamerTests/RVLCrossCheckTests.swift`
- `MikanXR/docs/rvl_test_vectors.json` — loaded by
  `src/Programs/Tests/UnitTests/arkit_rvl_real_capture_unit_tests.cpp`

Keep both copies in sync by hand whenever a new vector is added — there's no
build-time sync step, since this is a small, infrequently-updated fixture.

## Schema

A JSON array of objects, each shaped:

```json
{
  "description": "hand_closeup",
  "width": 256,
  "height": 192,
  "input": [0, 0, 1523, 1519, ...],
  "expected_compressed_hex": "40b3f2..."
}
```

- `description` — short, filesystem-friendly label for what the capture shows
  (e.g. `hand_closeup`, `flat_wall`, `cluttered_room`).
- `width`/`height` — always 256x192 for ARKit's LiDAR depth map
  (`kARKitDepthWidth`/`kARKitDepthHeight`), included for documentation/sanity-
  checking rather than because either test suite currently varies behavior by it.
- `input` — the raw depth plane in millimeters (`UInt16`, row-major, `0` =
  invalid), exactly as captured — i.e. the same array that gets RVL-encoded
  before being sent over the wire.
- `expected_compressed_hex` — the real on-device `rvlEncode(input)` output,
  lowercase hex, no separators. Both test suites decode this back to bytes and
  assert the round trip (`decode(encode(input)) == input`); the Swift suite
  additionally asserts `encode(input)` still matches this exact byte sequence,
  so a future accidental change to the encoder's bit-packing shows up here
  first.

## Capturing new vectors

1. Run `MikanARStreamer` on a real LiDAR-equipped device, connect a session
   with depth streaming enabled.
2. In the Settings screen's "Debug: RVL Test Vector Capture" section, type a
   description and tap "Capture Next Depth Frame" while pointing the camera at
   whatever scene you want captured.
3. Repeat for however many scenes are useful — a flat surface, a cluttered
   room, a close-up of a hand or other fast-moving/fine-detail subject (useful
   for edge cases near ARKit's confidence/range limits) are all good choices.
4. Pull `rvl_test_vectors.json` off the device via the Files app (On My
   iPhone/iPad > Mikan AR Streamer) — file sharing is enabled in `Info.plist`
   for exactly this purpose.
5. Copy the exported file over both repos' `docs/rvl_test_vectors.json`
   (replacing the whole file — the exporter already accumulates every capture
   from that device into one array).
6. Run both test suites; a mismatch means the two implementations disagree on
   a real input, which is exactly what this fixture exists to catch.

Both test suites treat an empty array as "not populated yet" and skip rather
than fail, so this file's presence alone (checked in, starting as `[]`) is
not itself a test failure.
