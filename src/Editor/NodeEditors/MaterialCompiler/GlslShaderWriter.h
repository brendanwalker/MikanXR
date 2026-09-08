#pragma once

#include "IShaderWriter.h"

// The GLSL 330 core backend. Its output matches the hand-written shaders in
// resources/shaders, so a compiled material reads like one.
class GlslShaderWriter : public IShaderWriter
{
public:
	GlslShaderWriter()= default;

	virtual std::string getLanguageName() const override;

	// -- Expressions -----
	virtual std::string typeName(eShaderValueType type) const override;
	virtual std::string literal(eShaderValueType type, const ShaderValueDefault& values) const override;
	virtual std::string broadcast(const std::string& scalarExpr, eShaderValueType toType) const override;
	virtual std::string constructVector(eShaderValueType type,
										const std::vector<std::string>& componentExprs) const override;
	virtual std::string swizzle(const std::string& expr, const std::string& mask) const override;
	virtual std::string binaryOp(eShaderBinaryOp op, const std::string& a, const std::string& b) const override;
	virtual std::string builtinCall(eShaderBuiltin builtin, const std::vector<std::string>& args,
									eShaderValueType argType) const override;
	virtual std::string sampleTexture(const std::string& samplerName, const std::string& uvExpr) const override;
	virtual std::string callFunction(const std::string& name, const std::vector<std::string>& args) const override;
	virtual std::string fragmentCoord() const override;

	// -- Declarations -----
	virtual std::string header(eShaderStage stage) const override;
	virtual std::string declareVertexInput(int location, eShaderValueType type, const std::string& name) const override;
	virtual std::string declareVarying(eShaderStage stage, eShaderValueType type,
									   const std::string& name) const override;
	virtual std::string declareUniform(eShaderValueType type, const std::string& name) const override;
	virtual std::string declareMatrixUniform(const std::string& name) const override;
	virtual std::string declareFragmentColorOutput() const override;
	virtual std::string declareFunction(eShaderValueType returnType, const std::string& name,
										const std::vector<ShaderFunctionParam>& params,
										const std::string& body) const override;

	// -- Statements (inside main) -----
	virtual std::string beginMain() const override;
	virtual std::string endMain() const override;
	virtual std::string declareTemp(eShaderValueType type, const std::string& name,
									const std::string& expr) const override;
	virtual std::string assignVarying(const std::string& name, const std::string& expr) const override;
	virtual std::string assignClipPosition2D(const std::string& positionExpr) const override;
	virtual std::string assignProjectedPosition(const std::string& matrixName,
												const std::string& positionExpr) const override;
	virtual std::string assignFragmentColor(const std::string& expr) const override;

private:
	// A float literal that always carries a decimal point, as GLSL requires
	static std::string floatLiteral(float value);
	static std::string joinArgs(const std::vector<std::string>& args);
};
