#pragma once

#include <string>

// The file extension of each node graph kind. A graph file also records its
// class inside, but the extension is what file dialogs, the asset catalog, and
// the drop targets filter on, so a compositor graph never lands on a shape.
namespace NodeGraphFileTypes
{
inline constexpr const char* k_compositorGraphExtension= ".compgraph";
inline constexpr const char* k_shapeGraphExtension= ".shapegraph";
inline constexpr const char* k_materialGraphExtension= ".matgraph";

inline constexpr const char* k_compositorGraphFilterPattern= "*.compgraph";
inline constexpr const char* k_shapeGraphFilterPattern= "*.shapegraph";
inline constexpr const char* k_materialGraphFilterPattern= "*.matgraph";

// The extension every graph kind shared before the split, still recognized on
// load so an old project migrates
inline constexpr const char* k_legacyGraphExtension= ".graph";

// Whether the path carries one of the graph extensions, legacy included
bool isGraphFileExtension(const std::string& extension);
} // namespace NodeGraphFileTypes
