#include "ShaderMathOpTable.h"

namespace
{
using MathRule= eShaderMathTypeRule;
using MathEmit= eShaderMathEmit;
using MathOp= eShaderMathOp;
using MathBin= eShaderBinaryOp;
using MathFn= eShaderBuiltin;

const eShaderValueType k_mathOpFloat3= eShaderValueType::float3;

ShaderMathOpInfo makeBinaryOpRow(MathOp op, const char* name, const char* titleKey, float defaultB, MathBin binaryOp)
{
	ShaderMathOpInfo info;
	info.op= op;
	info.name= name;
	info.titleKey= titleKey;
	info.inputs= {{"a", 0.f}, {"b", defaultB}};
	info.rule= MathRule::promote;
	info.emit= MathEmit::binaryOp;
	info.binaryOp= binaryOp;

	return info;
}

ShaderMathOpInfo makeBuiltinRow(MathOp op, const char* name, const char* titleKey,
								const std::vector<ShaderMathOpInput>& inputs, MathRule rule, MathFn builtin)
{
	ShaderMathOpInfo info;
	info.op= op;
	info.name= name;
	info.titleKey= titleKey;
	info.inputs= inputs;
	info.rule= rule;
	info.emit= MathEmit::builtin;
	info.builtin= builtin;

	return info;
}

ShaderMathOpInfo makeOneMinusRow()
{
	ShaderMathOpInfo info;
	info.op= MathOp::oneMinus;
	info.name= "oneMinus";
	info.titleKey= "nodes.mathOneMinusTitle";
	info.inputs= {{"value", 0.f}};
	info.rule= MathRule::promote;
	info.emit= MathEmit::oneMinus;
	info.binaryOp= MathBin::subtract;

	return info;
}

// Rows are indexed by the enumerator value, so the order must match eShaderMathOp
const ShaderMathOpInfo k_shaderMathOpTable[(int)MathOp::COUNT]= {
	makeBinaryOpRow(MathOp::add, "add", "nodes.mathAddTitle", 0.f, MathBin::add),
	makeBinaryOpRow(MathOp::subtract, "subtract", "nodes.mathSubtractTitle", 0.f, MathBin::subtract),
	makeBinaryOpRow(MathOp::multiply, "multiply", "nodes.mathMultiplyTitle", 1.f, MathBin::multiply),
	makeBinaryOpRow(MathOp::divide, "divide", "nodes.mathDivideTitle", 1.f, MathBin::divide),
	makeBuiltinRow(MathOp::lerp, "lerp", "nodes.mathLerpTitle", {{"a", 0.f}, {"b", 0.f}, {"alpha", 0.5f}},
				   MathRule::promote, MathFn::mix),
	makeBuiltinRow(MathOp::clamp, "clamp", "nodes.mathClampTitle", {{"value", 0.f}, {"min", 0.f}, {"max", 1.f}},
				   MathRule::promote, MathFn::clamp),
	makeBuiltinRow(MathOp::saturate, "saturate", "nodes.mathSaturateTitle", {{"value", 0.f}}, MathRule::promote,
				   MathFn::saturate),
	makeOneMinusRow(),
	makeBuiltinRow(MathOp::abs, "abs", "nodes.mathAbsTitle", {{"value", 0.f}}, MathRule::promote, MathFn::abs),
	makeBuiltinRow(MathOp::power, "power", "nodes.mathPowerTitle", {{"base", 0.f}, {"exponent", 2.f}},
				   MathRule::promote, MathFn::pow),
	makeBuiltinRow(MathOp::sqrt, "sqrt", "nodes.mathSqrtTitle", {{"value", 0.f}}, MathRule::promote, MathFn::sqrt),
	makeBuiltinRow(MathOp::sin, "sin", "nodes.mathSinTitle", {{"value", 0.f}}, MathRule::promote, MathFn::sin),
	makeBuiltinRow(MathOp::cos, "cos", "nodes.mathCosTitle", {{"value", 0.f}}, MathRule::promote, MathFn::cos),
	makeBuiltinRow(MathOp::fract, "fract", "nodes.mathFractTitle", {{"value", 0.f}}, MathRule::promote, MathFn::fract),
	makeBuiltinRow(MathOp::floor, "floor", "nodes.mathFloorTitle", {{"value", 0.f}}, MathRule::promote, MathFn::floor),
	makeBuiltinRow(MathOp::min, "min", "nodes.mathMinTitle", {{"a", 0.f}, {"b", 0.f}}, MathRule::promote, MathFn::min),
	makeBuiltinRow(MathOp::max, "max", "nodes.mathMaxTitle", {{"a", 0.f}, {"b", 0.f}}, MathRule::promote, MathFn::max),
	makeBuiltinRow(MathOp::step, "step", "nodes.mathStepTitle", {{"edge", 0.f}, {"value", 0.f}}, MathRule::promote,
				   MathFn::step),
	makeBuiltinRow(MathOp::smoothstep, "smoothstep", "nodes.mathSmoothstepTitle",
				   {{"min", 0.f}, {"max", 1.f}, {"value", 0.f}}, MathRule::promote, MathFn::smoothstep),
	makeBuiltinRow(MathOp::dot, "dot", "nodes.mathDotTitle", {{"a", 0.f}, {"b", 0.f}}, MathRule::scalarResult,
				   MathFn::dot),
	makeBuiltinRow(MathOp::cross, "cross", "nodes.mathCrossTitle",
				   {{"a", 0.f, k_mathOpFloat3}, {"b", 0.f, k_mathOpFloat3}}, MathRule::float3Result, MathFn::cross),
	makeBuiltinRow(MathOp::normalize, "normalize", "nodes.mathNormalizeTitle", {{"value", 0.f}}, MathRule::promote,
				   MathFn::normalize),
	makeBuiltinRow(MathOp::length, "length", "nodes.mathLengthTitle", {{"value", 0.f}}, MathRule::scalarResult,
				   MathFn::length),
	makeBuiltinRow(MathOp::distance, "distance", "nodes.mathDistanceTitle", {{"a", 0.f}, {"b", 0.f}},
				   MathRule::scalarResult, MathFn::distance),
};

const ShaderMathOpInfo k_invalidShaderMathOp= {};
const std::string k_invalidShaderMathOpName= "INVALID";

std::vector<eShaderMathOp> buildAllShaderMathOps()
{
	std::vector<eShaderMathOp> ops;
	ops.reserve((size_t)MathOp::COUNT);
	for (int index= 0; index < (int)MathOp::COUNT; ++index)
	{
		ops.push_back((MathOp)index);
	}

	return ops;
}
} // namespace

namespace ShaderMathOpTable
{
const ShaderMathOpInfo& getOpInfo(eShaderMathOp op)
{
	const int index= (int)op;
	return (index > (int)MathOp::INVALID && index < (int)MathOp::COUNT) ? k_shaderMathOpTable[index]
																		: k_invalidShaderMathOp;
}

const std::string& getOpName(eShaderMathOp op)
{
	const int index= (int)op;
	return (index > (int)MathOp::INVALID && index < (int)MathOp::COUNT) ? k_shaderMathOpTable[index].name
																		: k_invalidShaderMathOpName;
}

eShaderMathOp opFromName(const std::string& name)
{
	for (int index= 0; index < (int)MathOp::COUNT; ++index)
	{
		if (k_shaderMathOpTable[index].name == name)
			return (MathOp)index;
	}

	return MathOp::INVALID;
}

const std::vector<eShaderMathOp>& getAllOps()
{
	static const std::vector<eShaderMathOp> k_allOps= buildAllShaderMathOps();
	return k_allOps;
}
} // namespace ShaderMathOpTable
