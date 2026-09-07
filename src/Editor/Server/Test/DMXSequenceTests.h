#pragma once

// Validates the DMX sequence pieces that need no project:
//   - the definition round-trips through JSON, including the content source
//     settings, and an old project loads as the script-driven sequence it was
//   - advanceTime wraps when looping, clamps and finishes otherwise, and
//     leaves an unbounded sequence alone
//   - the frame buffer writers grow and overwrite slices as expected
//   - the scroll offset sweeps the content fully through the grid and wraps on
//     the same period when looping
//   - the animation frame index follows the per-frame delays and the speed
//     scale, wrapping when looping and holding the last frame otherwise
//   - a sprite sheet splits into frames left to right, then top to bottom
//   - UTF-8 decoding handles every sequence length and never stalls on bad input
//   - a canvas window blits into a grid at each cell's wire position, with the
//     background wherever the content does not reach
bool run_dmx_sequence_tests();
