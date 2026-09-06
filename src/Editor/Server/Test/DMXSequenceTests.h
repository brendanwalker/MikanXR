#pragma once

// Validates the DMX sequence pieces that need no project:
//   - the definition round-trips through JSON
//   - advanceTime wraps when looping, clamps and finishes otherwise, and
//     leaves an unbounded sequence alone
//   - the frame buffer writers grow and overwrite slices as expected
bool run_dmx_sequence_tests();
