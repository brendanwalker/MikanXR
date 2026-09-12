#pragma once

// Validates the project asset catalog against a temp project on disk:
//   - the scan lists each folder's files by type, one tile per .mat, and the
//     bundled fonts as a read-only overlay
//   - import copies into the folder with a numeric suffix on collision, leaves a
//     file already inside the folder alone, and refuses unsupported types
//   - material import lands the folder under shaders/<domain>/ and refuses a
//     duplicate or a .mat with no domain
//   - findReferences names the graphs, materials, and texture parameter nodes
//     that use a path, and not a material's own sources
//   - delete removes a file or a material folder and refuses bundled entries
//   - AssetReference stores project-relative paths and writes them as UTF-8
bool run_project_asset_catalog_tests();
