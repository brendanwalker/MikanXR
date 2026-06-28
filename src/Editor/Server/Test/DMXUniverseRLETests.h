#pragma once

// Validates the DMX universe RLE codec (mikanRLEEncodeDMXUniverseBuffer /
// mikanRLEDecodeDMXUniverseBuffer) declared in MikanLightTypes.h:
//   - typical DMX universes (long runs of zeros) round-trip and compress
//   - runs longer than the 255 max run length split correctly
//   - incompressible input falls back to a Raw buffer no larger than the source
//   - decode honors buffer_format (Raw copied through, RLEEncoded expanded)
//   - decode clamps its output to out_buffer_max_size
bool run_dmx_universe_rle_tests();
