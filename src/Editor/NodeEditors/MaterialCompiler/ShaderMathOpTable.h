#pragma once

#include "IShaderWriter.h"
#include "ShaderValueType.h"

#include <string>
#include <vector>

// Every operation the math node offers. The persisted name is the enumerator
// spelled in lowerCamel; adding an op means adding a row to the table in
// ShaderMathOpTable.cpp and a nodes.math<Op>Title localization key.
enum class eShaderMathOp : int
{
	INVALID= -1,

	add,
	subtract,
	multiply,
	divide,
	lerp,
	clamp,
	saturate,
	oneMinus,
	abs,
	power,
	sqrt,
	sin,
	cos,
	fract,
	floor,
	min,
	max,
	step,
	smoothstep,
	dot,
	cross,
	normalize,
	length,
	distance,

	COUNT
};

// How an op's result type follows from its operand types
enum class eShaderMathTypeRule : int
{
	// Fold every operand through ShaderValueTypeUtils::promote and coerce them all to the result
	promote,
	// Operands promote as above, the result is a float1 (dot, length, distance)
	scalarResult,
	// Operands and result are float3 (cross)
	float3Result
};

// How an op's expression is spelled through the writer
enum class eShaderMathEmit : int
{
	binaryOp,
	// 1 - x, spelled as a subtract against a literal of the result type
	oneMinus,
	builtin
};

struct ShaderMathOpInput
{
	std::string name;
	float defaultValue= 0.f;
	// Wildcard for every op except cross, whose operands are float3
	eShaderValueType declaredType= eShaderValueType::wildcard;
};

struct ShaderMathOpInfo
{
	eShaderMathOp op= eShaderMathOp::INVALID;
	// Persisted in the node config, never shown
	std::string name;
	// Localization key of the node title
	std::string titleKey;
	std::vector<ShaderMathOpInput> inputs;
	eShaderMathTypeRule rule= eShaderMathTypeRule::promote;
	eShaderMathEmit emit= eShaderMathEmit::builtin;
	// Used when emit is binaryOp
	eShaderBinaryOp binaryOp= eShaderBinaryOp::add;
	// Used when emit is builtin
	eShaderBuiltin builtin= eShaderBuiltin::abs;
};

namespace ShaderMathOpTable
{
const ShaderMathOpInfo& getOpInfo(eShaderMathOp op);
const std::string& getOpName(eShaderMathOp op);
eShaderMathOp opFromName(const std::string& name);
const std::vector<eShaderMathOp>& getAllOps();
} // namespace ShaderMathOpTable
