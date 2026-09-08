#pragma once

#include "ShaderValueType.h"

#include <string>
#include <utility>
#include <vector>

enum class eShaderStage : int
{
	vertex,
	fragment
};

enum class eShaderBinaryOp : int
{
	add,
	subtract,
	multiply,
	divide
};

// Builtin functions a writer spells in its own language. Nodes name the
// builtin, never the function text, so a second language is one new writer.
enum class eShaderBuiltin : int
{
	mix,
	clamp,
	saturate,
	fract,
	floor,
	ceil,
	round,
	abs,
	sign,
	pow,
	sqrt,
	exp,
	log,
	sin,
	cos,
	tan,
	atan2,
	min,
	max,
	step,
	smoothstep,
	mod,
	dot,
	cross,
	normalize,
	length,
	distance
};

struct ShaderFunctionParam
{
	eShaderValueType type;
	std::string name;
};

// The language backend of the material compiler. Every method returns text;
// declaration methods return one complete line, expression methods return an
// expression with no trailing terminator.
class IShaderWriter
{
public:
	virtual ~IShaderWriter()= default;

	virtual std::string getLanguageName() const= 0;

	// -- Expressions -----
	virtual std::string typeName(eShaderValueType type) const= 0;
	virtual std::string literal(eShaderValueType type, const ShaderValueDefault& values) const= 0;
	// Widen a scalar expression to a vector type, as in vec3(x)
	virtual std::string broadcast(const std::string& scalarExpr, eShaderValueType toType) const= 0;
	virtual std::string constructVector(eShaderValueType type, const std::vector<std::string>& componentExprs) const= 0;
	virtual std::string swizzle(const std::string& expr, const std::string& mask) const= 0;
	virtual std::string binaryOp(eShaderBinaryOp op, const std::string& a, const std::string& b) const= 0;
	// argType is the operand type, for builtins whose spelling depends on it (saturate)
	virtual std::string builtinCall(eShaderBuiltin builtin, const std::vector<std::string>& args,
									eShaderValueType argType) const= 0;
	virtual std::string sampleTexture(const std::string& samplerName, const std::string& uvExpr) const= 0;
	virtual std::string callFunction(const std::string& name, const std::vector<std::string>& args) const= 0;
	// Window-space fragment position, float2
	virtual std::string fragmentCoord() const= 0;

	// -- Declarations -----
	virtual std::string header(eShaderStage stage) const= 0;
	virtual std::string declareVertexInput(int location, eShaderValueType type, const std::string& name) const= 0;
	virtual std::string declareVarying(eShaderStage stage, eShaderValueType type, const std::string& name) const= 0;
	virtual std::string declareUniform(eShaderValueType type, const std::string& name) const= 0;
	virtual std::string declareMatrixUniform(const std::string& name) const= 0;
	virtual std::string declareFragmentColorOutput() const= 0;
	virtual std::string declareFunction(eShaderValueType returnType, const std::string& name,
										const std::vector<ShaderFunctionParam>& params,
										const std::string& body) const= 0;

	// -- Statements (inside main) -----
	virtual std::string beginMain() const= 0;
	virtual std::string endMain() const= 0;
	virtual std::string declareTemp(eShaderValueType type, const std::string& name, const std::string& expr) const= 0;
	virtual std::string assignVarying(const std::string& name, const std::string& expr) const= 0;
	// gl_Position from a float2 clip-space position (the compositor quad)
	virtual std::string assignClipPosition2D(const std::string& positionExpr) const= 0;
	// gl_Position from a float3 object-space position through the named matrix uniform
	virtual std::string assignProjectedPosition(const std::string& matrixName,
												const std::string& positionExpr) const= 0;
	virtual std::string assignFragmentColor(const std::string& expr) const= 0;
};
