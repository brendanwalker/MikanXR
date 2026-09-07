#pragma once

// Validates RGBPixelGridDefinition's layout math, the mapping between a
// physical grid cell and its position in the DMX stream:
//   - every origin, with and without zig-zag, covers each cell exactly once
//     and round-trips through getPixelGridPosition
//   - upper left with no zig-zag stays row * columns + col, the mapping that
//     was hardcoded before the layout properties existed
//   - zig-zag reverses only the odd scan rows
//   - pixel centres are symmetric about the component origin and their span
//     matches getGridLocalHalfExtents
bool run_pixel_grid_layout_tests();
