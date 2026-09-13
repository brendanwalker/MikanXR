#pragma once

// Validates the one-time move of a project from the shared graphs/ folder and
// the .graph extension onto per-kind folders and extensions:
//   - a legacy compositor graph lands in compositors/ as .compgraph and a shape
//     graph in shapes/ as .shapegraph, by the class each file records
//   - a material graph beside its .mat becomes .matgraph and the .mat follows
//   - the compositor and shape graph paths in the project file are rewritten,
//     including bundled references whose files never lived in the project
//   - a project already on the current layout is left untouched
bool run_legacy_graph_migration_tests();
