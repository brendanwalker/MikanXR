#pragma once

#include "NodeFwd.h"

// Registers every material graph node factory (constants, parameters, vertex
// and semantic inputs, texture sampling, the math table, swizzle, append,
// custom expression) on a MaterialNodeGraph. The output node is registered by
// the graph itself.
namespace MaterialNodeLibrary
{
void registerNodeFactories(NodeGraph& graph);
}
