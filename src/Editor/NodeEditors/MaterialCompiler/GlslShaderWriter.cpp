#include "GlslShaderWriter.h"

#include <cctype>
#include <cstdio>
#include <sstream>

namespace
{
const char* k_tab= "\t";

bool isPlainIdentifier(const std::string& expr)
{
	if (expr.empty())
		return false;

	for (char c : expr)
	{
		if (!std::isalnum((unsigned char)c) && c != '_')
			return false;
	}

	return true;
}

const char* builtinName(eShaderBuiltin builtin)
{
	switch (builtin)
	{
	case eShaderBuiltin::mix:
		return "mix";
	case eShaderBuiltin::clamp:
		return "clamp";
	case eShaderBuiltin::saturate:
		return "clamp";
	case eShaderBuiltin::fract:
		return "fract";
	case eShaderBuiltin::floor:
		return "floor";
	case eShaderBuiltin::ceil:
		return "ceil";
	case eShaderBuiltin::round:
		return "round";
	case eShaderBuiltin::abs:
		return "abs";
	case eShaderBuiltin::sign:
		return "sign";
	case eShaderBuiltin::pow:
		return "pow";
	case eShaderBuiltin::sqrt:
		return "sqrt";
	case eShaderBuiltin::exp:
		return "exp";
	case eShaderBuiltin::log:
		return "log";
	case eShaderBuiltin::sin:
		return "sin";
	case eShaderBuiltin::cos:
		return "cos";
	case eShaderBuiltin::tan:
		return "tan";
	case eShaderBuiltin::atan2:
		return "atan";
	case eShaderBuiltin::min:
		return "min";
	case eShaderBuiltin::max:
		return "max";
	case eShaderBuiltin::step:
		return "step";
	case eShaderBuiltin::smoothstep:
		return "smoothstep";
	case eShaderBuiltin::mod:
		return "mod";
	case eShaderBuiltin::dot:
		return "dot";
	case eShaderBuiltin::cross:
		return "cross";
	case eShaderBuiltin::normalize:
		return "normalize";
	case eShaderBuiltin::length:
		return "length";
	case eShaderBuiltin::distance:
		return "distance";
	default:
		return "";
	}
}
} // namespace

std::string GlslShaderWriter::getLanguageName() const { return "glsl330"; }

// -- Expressions -----
std::string GlslShaderWriter::typeName(eShaderValueType type) const
{
	switch (type)
	{
	case eShaderValueType::float2:
		return "vec2";
	case eShaderValueType::float3:
		return "vec3";
	case eShaderValueType::float4:
		return "vec4";
	case eShaderValueType::texture2D:
		return "sampler2D";
	case eShaderValueType::float1:
	default:
		return "float";
	}
}

std::string GlslShaderWriter::literal(eShaderValueType type, const ShaderValueDefault& values) const
{
	const int componentCount= ShaderValueTypeUtils::getComponentCount(type);
	if (componentCount <= 1)
	{
		return floatLiteral(values[0]);
	}

	std::vector<std::string> componentExprs;
	for (int index= 0; index < componentCount; ++index)
	{
		componentExprs.push_back(floatLiteral(values[index]));
	}

	return constructVector(type, componentExprs);
}

std::string GlslShaderWriter::broadcast(const std::string& scalarExpr, eShaderValueType toType) const
{
	if (ShaderValueTypeUtils::getComponentCount(toType) <= 1)
		return scalarExpr;

	return typeName(toType) + "(" + scalarExpr + ")";
}

std::string GlslShaderWriter::constructVector(eShaderValueType type,
											  const std::vector<std::string>& componentExprs) const
{
	return typeName(type) + "(" + joinArgs(componentExprs) + ")";
}

std::string GlslShaderWriter::swizzle(const std::string& expr, const std::string& mask) const
{
	return isPlainIdentifier(expr) ? expr + "." + mask : "(" + expr + ")." + mask;
}

std::string GlslShaderWriter::binaryOp(eShaderBinaryOp op, const std::string& a, const std::string& b) const
{
	const char* opText= "+";
	switch (op)
	{
	case eShaderBinaryOp::subtract:
		opText= "-";
		break;
	case eShaderBinaryOp::multiply:
		opText= "*";
		break;
	case eShaderBinaryOp::divide:
		opText= "/";
		break;
	case eShaderBinaryOp::add:
	default:
		break;
	}

	return "(" + a + " " + opText + " " + b + ")";
}

std::string GlslShaderWriter::builtinCall(eShaderBuiltin builtin, const std::vector<std::string>& args,
										  eShaderValueType argType) const
{
	if (builtin == eShaderBuiltin::saturate)
	{
		const std::string x= args.empty() ? std::string() : args[0];
		const std::string zero= broadcast("0.0", argType);
		const std::string one= broadcast("1.0", argType);

		return "clamp(" + x + ", " + zero + ", " + one + ")";
	}

	return std::string(builtinName(builtin)) + "(" + joinArgs(args) + ")";
}

std::string GlslShaderWriter::sampleTexture(const std::string& samplerName, const std::string& uvExpr) const
{
	return "texture(" + samplerName + ", " + uvExpr + ")";
}

std::string GlslShaderWriter::callFunction(const std::string& name, const std::vector<std::string>& args) const
{
	return name + "(" + joinArgs(args) + ")";
}

std::string GlslShaderWriter::fragmentCoord() const { return "gl_FragCoord.xy"; }

// -- Declarations -----
std::string GlslShaderWriter::header(eShaderStage stage) const { return "#version 330 core"; }

std::string GlslShaderWriter::declareVertexInput(int location, eShaderValueType type, const std::string& name) const
{
	return "layout (location = " + std::to_string(location) + ") in " + typeName(type) + " " + name + ";";
}

std::string GlslShaderWriter::declareVarying(eShaderStage stage, eShaderValueType type, const std::string& name) const
{
	const char* qualifier= (stage == eShaderStage::vertex) ? "out" : "in";

	return std::string(qualifier) + " " + typeName(type) + " " + name + ";";
}

std::string GlslShaderWriter::declareUniform(eShaderValueType type, const std::string& name) const
{
	return "uniform " + typeName(type) + " " + name + ";";
}

std::string GlslShaderWriter::declareMatrixUniform(const std::string& name) const
{
	return "uniform mat4 " + name + ";";
}

std::string GlslShaderWriter::declareFragmentColorOutput() const { return "out vec4 FragColor;"; }

std::string GlslShaderWriter::declareFunction(eShaderValueType returnType, const std::string& name,
											  const std::vector<ShaderFunctionParam>& params,
											  const std::string& body) const
{
	std::string text= typeName(returnType) + " " + name + "(";
	for (size_t index= 0; index < params.size(); ++index)
	{
		if (index > 0)
		{
			text+= ", ";
		}
		text+= typeName(params[index].type) + " " + params[index].name;
	}
	text+= ")\n{\n";

	// Indent the user's body one level, leaving blank lines blank
	std::istringstream bodyStream(body);
	std::string line;
	while (std::getline(bodyStream, line))
	{
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}
		if (!line.empty())
		{
			text+= k_tab;
			text+= line;
		}
		text+= "\n";
	}
	text+= "}";

	return text;
}

// -- Statements -----
std::string GlslShaderWriter::beginMain() const { return "void main()\n{"; }

std::string GlslShaderWriter::endMain() const { return "}"; }

std::string GlslShaderWriter::declareTemp(eShaderValueType type, const std::string& name, const std::string& expr) const
{
	return k_tab + typeName(type) + " " + name + " = " + expr + ";";
}

std::string GlslShaderWriter::assignVarying(const std::string& name, const std::string& expr) const
{
	return k_tab + name + " = " + expr + ";";
}

std::string GlslShaderWriter::assignClipPosition2D(const std::string& positionExpr) const
{
	return k_tab + std::string("gl_Position = vec4(") + positionExpr + ", 0.0, 1.0);";
}

std::string GlslShaderWriter::assignProjectedPosition(const std::string& matrixName,
													  const std::string& positionExpr) const
{
	return k_tab + std::string("gl_Position = ") + matrixName + " * vec4(" + positionExpr + ", 1.0);";
}

std::string GlslShaderWriter::assignFragmentColor(const std::string& expr) const
{
	return k_tab + std::string("FragColor = ") + expr + ";";
}

// -- Helpers -----
std::string GlslShaderWriter::floatLiteral(float value)
{
	char buffer[64];
	std::snprintf(buffer, sizeof(buffer), "%.7g", (double)value);
	std::string text= buffer;

	// %g drops trailing zeros and the point for whole values; GLSL wants the point back
	if (text.find('.') == std::string::npos)
	{
		const size_t exponentPos= text.find_first_of("eE");
		if (exponentPos == std::string::npos)
		{
			text+= ".0";
		}
		else
		{
			text.insert(exponentPos, ".0");
		}
	}

	return text;
}

std::string GlslShaderWriter::joinArgs(const std::vector<std::string>& args)
{
	std::string text;
	for (size_t index= 0; index < args.size(); ++index)
	{
		if (index > 0)
		{
			text+= ", ";
		}
		text+= args[index];
	}

	return text;
}
