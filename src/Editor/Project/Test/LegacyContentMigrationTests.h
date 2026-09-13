#pragma once

// Validates the one-time move of a project from the shared graphs/ and shaders/
// folders and the .graph and .mat extensions onto per-kind folders and extensions:
//   - a legacy compositor graph lands in compositors/ as .compgraph and a shape
//     graph in shapes/ as .shapegraph, by the class each file records
//   - a material folder moves under compositor_materials/ or shape_materials/
//     by its domain, its material file renamed to .compmat or .shapemat, its
//     graph to .matgraph, and the material file follows the graph rename
//   - the compositor and shape graph paths in the project file are rewritten,
//     including bundled references whose files never lived in the project
//   - the material references inside compositor and shape graph files are
//     rewritten to the domain class and path
//   - a project already on the current layout is left untouched
bool run_legacy_content_migration_tests();
