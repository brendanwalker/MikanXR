#pragma once

#include "MikanShaderConfig.h"
#include "Graphs/NodeError.h"
#include "ShaderValueType.h"

#include <memory>
#include <string>
#include <vector>

// A parameter node's default, carried out of the compile so the preview can
// feed the uniform without evaluating the graph a second time
struct MaterialParameterDefault
{
	std::string name;
	eShaderValueType type= eShaderValueType::INVALID;
	ShaderValueDefault value= {};
	// Texture parameters only: the default texture asset path, empty for none
	std::string textureAssetPath;
};

struct MaterialCompileResult
{
	std::string vertexSource;
	std::string fragmentSource;

	// Attributes, uniform map, domain and preset are filled by the compiler.
	// The shader paths, material name and source graph path are filled when
	// the outputs are written beside a graph file. Held by pointer because a
	// CommonConfig cannot be copied and a result is passed by value.
	std::shared_ptr<MikanShaderConfig> config= std::make_shared<MikanShaderConfig>();

	std::vector<NodeEvaluationError> errors;

	// Set when a custom expression node contributed raw shader text
	bool glslOnly= false;

	std::vector<MaterialParameterDefault> parameterDefaults;

	bool hasErrors() const { return !errors.empty(); }
};
