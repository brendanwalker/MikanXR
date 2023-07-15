#include "GlCommon.h"
#include "GlProgram.h"
#include "GlProgramConstants.h"
#include "GlTexture.h"
#include "Logger.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <assert.h>

// -- GlProgramCode -----
GlProgramCode::GlProgramCode(
	const std::string& programName,
	const std::string& vertexCode, 
	const std::string& fragmentCode)
	: m_programName(programName)
	, m_vertexShaderCode(vertexCode)
	, m_framementShaderCode(fragmentCode)
{
	std::hash<std::string> hasher;

	m_shaderCodeHash= hasher(vertexCode + fragmentCode);
}

bool GlProgramCode::loadFromConfigData(
	const std::filesystem::path& shaderConfigPath,
	const std::filesystem::path& vertexShaderFileName,
	const std::filesystem::path& fragmentShaderFileName,
	const std::map<std::string, std::string>& uniforms)
{
	bool bSuccess= true;

	m_programName = shaderConfigPath.string();
	
	std::filesystem::path shaderFolderPath = m_programName;
	shaderFolderPath.remove_filename();

	try
	{
		std::filesystem::path vertexShaderPath = shaderFolderPath;
		vertexShaderPath/= vertexShaderFileName;

		std::ifstream t(vertexShaderPath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		m_vertexShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("GlProgramCode::loadFromConfigData")
			<< m_programName
			<< " - unable to load vertex shader file!";
		bSuccess= false;
	}

	try
	{
		std::filesystem::path fragmentShaderPath = shaderFolderPath;
		fragmentShaderPath /= fragmentShaderFileName;

		std::ifstream t(fragmentShaderPath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		m_framementShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("GlProgramCode::loadFromConfigData")
			<< m_programName
			<< " - unable to load fragment shader file!";
		bSuccess = false;
	}

	for (const auto& [name, semanticString] : uniforms)
	{
		eUniformSemantic semantic= eUniformSemantic::INVALID;
		for (int enumIntValue = 0; enumIntValue < (int)eUniformSemantic::COUNT; ++enumIntValue)
		{
			if (k_UniformSemanticName[enumIntValue] == semanticString)
			{
				semantic= (eUniformSemantic)enumIntValue;
				break;
			}
		}

		if (semantic != eUniformSemantic::INVALID)
		{
			m_uniformList.push_back({name, semantic});
		}
		else
		{
			MIKAN_LOG_ERROR("GlProgramCode::loadFromConfigData")
				<< "Invalid semantic: "
				<< name
				<< " -> "
				<< semanticString;
			bSuccess = false;
		}
	}

	if (bSuccess)
	{
		std::hash<std::string> hasher;

		m_shaderCodeHash = hasher(m_vertexShaderCode + m_framementShaderCode);
	}

	return bSuccess;
}

// -- GlProgram -----
GlProgram::GlProgram(const GlProgramCode& code)
	: m_code(code)
{
	m_code = code;
}

GlProgram::~GlProgram()
{
	deleteProgram();
}

bool GlProgram::getUniformSemantic(const std::string uniformName, eUniformSemantic& outSemantic) const
{
	auto it= m_uniformLocationMap.find(uniformName);
	if (it != m_uniformLocationMap.end())
	{
		outSemantic= it->second.semantic;
		return true;
	}

	return false;
}

eUniformDataType GlProgram::getUniformSemanticDataType(eUniformSemantic semantic)
{
	eUniformDataType dataType= eUniformDataType::INVALID;

	switch (semantic)
	{
		case eUniformSemantic::transformMatrix:
		case eUniformSemantic::modelMatrix:
		case eUniformSemantic::normalMatrix:
		case eUniformSemantic::viewMatrix:
		case eUniformSemantic::projectionMatrix:
		case eUniformSemantic::modelViewProjectionMatrix:
			dataType= eUniformDataType::datatype_mat4;
			break;
		case eUniformSemantic::ambientColorRGBA:
		case eUniformSemantic::diffuseColorRGBA:
		case eUniformSemantic::specularColorRGBA:
		case eUniformSemantic::lightColor:
			dataType= eUniformDataType::datatype_float4;
			break;
		case eUniformSemantic::diffuseColorRGB:
		case eUniformSemantic::cameraPosition:
		case eUniformSemantic::lightDirection:
			dataType= eUniformDataType::datatype_float3;
			break;
		case eUniformSemantic::screenPosition:
			dataType= eUniformDataType::datatype_float2;
			break;
		case eUniformSemantic::shininess:
		case eUniformSemantic::floatConstant0:
		case eUniformSemantic::floatConstant1:
		case eUniformSemantic::floatConstant2:
		case eUniformSemantic::floatConstant3:
			dataType = eUniformDataType::datatype_float;
			break;
		case eUniformSemantic::texture0:
		case eUniformSemantic::texture1:
		case eUniformSemantic::texture2:
		case eUniformSemantic::texture3:
		case eUniformSemantic::texture4:
		case eUniformSemantic::texture5:
		case eUniformSemantic::texture6:
		case eUniformSemantic::texture7:
		case eUniformSemantic::texture8:
		case eUniformSemantic::texture9:
		case eUniformSemantic::texture10:
		case eUniformSemantic::texture11:
		case eUniformSemantic::texture12:
		case eUniformSemantic::texture13:
		case eUniformSemantic::texture14:
		case eUniformSemantic::texture15:
		case eUniformSemantic::texture16:
		case eUniformSemantic::texture17:
		case eUniformSemantic::texture18:
		case eUniformSemantic::texture19:
		case eUniformSemantic::texture20:
		case eUniformSemantic::texture21:
		case eUniformSemantic::texture22:
		case eUniformSemantic::texture23:
		case eUniformSemantic::texture24:
		case eUniformSemantic::texture25:
		case eUniformSemantic::texture26:
		case eUniformSemantic::texture27:
		case eUniformSemantic::texture28:
		case eUniformSemantic::texture29:
		case eUniformSemantic::texture30:
		case eUniformSemantic::texture31:
			dataType= eUniformDataType::datatype_texture;
			break;
		default:
			assert(false);
	}

	return dataType;
}

bool GlProgram::getUniformDataType(const std::string uniformName, eUniformDataType& outDataType) const
{
	eUniformSemantic semantic = eUniformSemantic::INVALID;
	if (getUniformSemantic(uniformName, semantic))
	{
		outDataType= GlProgram::getUniformSemanticDataType(semantic);
		return true;
	}

	return false;
}

std::vector<std::string> GlProgram::getUniformNamesOfDataType(const eUniformDataType dataType) const
{
	std::vector<std::string> uniformNames;

	for (auto it = m_uniformLocationMap.begin(); it != m_uniformLocationMap.end(); it++)
	{
		const eUniformSemantic uniformSemantic= it->second.semantic;
		const eUniformDataType uniformDataType= getUniformSemanticDataType(uniformSemantic);

		if (uniformDataType == dataType)
		{
			uniformNames.push_back(it->first);
		}
	}

	return uniformNames;
}

bool GlProgram::getFirstUniformNameOfSemantic(eUniformSemantic semantic, std::string& outUniformName) const
{
	for (auto it = getUniformBegin(); it != getUniformEnd(); ++it)
	{
		if (it->second.semantic == semantic)
		{
			outUniformName= it->first;
			return true;
		}
	}

	return false;
}

bool GlProgram::getTextureUniformUnit(eUniformSemantic semantic, int& outTextureUnit)
{
	const int enumValue = (int)semantic;
	const int startTextureIndex = (int)eUniformSemantic::texture0;
	const int endTextureIndex = (int)eUniformSemantic::texture31;

	if (enumValue >= startTextureIndex && enumValue < endTextureIndex)
	{
		outTextureUnit= enumValue - startTextureIndex;
		return true;
	}

	return false;
}

bool GlProgram::getUniformTextureUnit(const std::string uniformName, int& outTextureUnit) const
{
	eUniformSemantic semantic = eUniformSemantic::INVALID;
	if (getUniformSemantic(uniformName, semantic) &&
		getTextureUniformUnit(semantic, outTextureUnit))
	{
		return true;
	}

	return false;
}

bool GlProgram::setMatrix4x4Uniform(
	const std::string uniformName,
	const glm::mat4& mat)
{
	auto iter= m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(mat));
		return !checkHasAnyGLError("GlProgram::setMatrix4x4Uniform()", __FILE__, __LINE__);
	}
	return false;
}

bool GlProgram::setFloatUniform(
	const std::string uniformName,
	const float value)
{
	auto iter = m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform1f(uniformId, value);
		return !checkHasAnyGLError("GlProgram::setFloatUniform()", __FILE__, __LINE__);
	}
	return false;
}

bool GlProgram::setVector2Uniform(
	const std::string uniformName,
	const glm::vec2& vec)
{
	auto iter = m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform2fv(uniformId, 1, glm::value_ptr(vec));
		return !checkHasAnyGLError("GlProgram::setVector2Uniform()", __FILE__, __LINE__);
	}
	return false;
}

bool GlProgram::setVector3Uniform(
	const std::string uniformName,
	const glm::vec3& vec)
{
	auto iter = m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform3fv(uniformId, 1, glm::value_ptr(vec));		
		return !checkHasAnyGLError("GlProgram::setVector3Uniform()", __FILE__, __LINE__);
	}
	return false;
}

bool GlProgram::setVector4Uniform(
	const std::string uniformName,
	const glm::vec4& vec)
{
	auto iter = m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform4fv(uniformId, 1, glm::value_ptr(vec));
		return !checkHasAnyGLError("GlProgram::setVector4Uniform()", __FILE__, __LINE__);
	}
	return false;
}

bool GlProgram::setTextureUniform(
	const std::string uniformName)
{
	auto iter = m_uniformLocationMap.find(uniformName);
	if (iter != m_uniformLocationMap.end())
	{
		GLint textureUnit;
		if (getTextureUniformUnit(iter->second.semantic, textureUnit))
		{
			const GLint uniformId = iter->second.locationId;

			glUniform1i(uniformId, textureUnit);
			return !checkHasAnyGLError("GlProgram::setTextureUniform()", __FILE__, __LINE__);
		}
	}

	return false;
}

bool GlProgram::createProgram()
{
	if (m_code.hasCode())
	{
		m_programID = glCreateProgram();

		uint32_t nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
		const GLchar* vertexShaderSource = (const GLchar*)m_code.getVertexShaderCode();
		glShaderSource(nSceneVertexShader, 1, &vertexShaderSource, nullptr);
		glCompileShader(nSceneVertexShader);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		int vShaderCompiled = 0;
		glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		if (vShaderCompiled != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::createProgram")
				<< m_code.getProgramName()
				<< " - Unable to compile vertex shader "
				<< nSceneVertexShader;

			GLchar strInfoLog[1024] = { 0 };
			glGetShaderInfoLog(nSceneVertexShader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
			MIKAN_LOG_ERROR("GlProgram::createProgram") << strInfoLog;

			glDeleteProgram(m_programID);
			glDeleteShader(nSceneVertexShader);
			m_programID = 0;

			return false;
		}
		glAttachShader(m_programID, nSceneVertexShader);
		glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

		uint32_t nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const GLchar* fragmentShaderSource = (const GLchar*)m_code.getFragmentShaderCode();
		glShaderSource(nSceneFragmentShader, 1, &fragmentShaderSource, nullptr);
		glCompileShader(nSceneFragmentShader);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		int fShaderCompiled = 0;
		glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		if (fShaderCompiled != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::CreateGLResources")
				<< m_code.getProgramName()
				<< " - Unable to compile fragment shader "
				<< nSceneFragmentShader;

			GLchar strInfoLog[1024] = { 0 };
			glGetShaderInfoLog(nSceneFragmentShader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
			MIKAN_LOG_ERROR("GlProgram::createProgram") << strInfoLog;

			glDeleteProgram(m_programID);
			glDeleteShader(nSceneFragmentShader);
			m_programID = 0;

			return false;
		}
		glAttachShader(m_programID, nSceneFragmentShader);
		glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

		glLinkProgram(m_programID);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		int programSuccess = 1;
		glGetProgramiv(m_programID, GL_LINK_STATUS, &programSuccess);
		checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

		if (programSuccess != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::CreateGLResources")
				<< m_code.getProgramName()
				<< " - Error linking program "
				<< m_programID;

			GLchar strInfoLog[1024] = { 0 };
			glGetProgramInfoLog(m_programID, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
			MIKAN_LOG_ERROR("GlProgram::createProgram") << strInfoLog;

			glDeleteProgram(m_programID);
			m_programID = 0;

			return false;
		}

		for (const GlProgramCode::Uniform& codeUniform : m_code.getUniformList())
		{
			GLint uniformId = glGetUniformLocation(m_programID, codeUniform.name.c_str());
			checkHasAnyGLError("GlProgram::createProgram()", __FILE__, __LINE__);

			if (uniformId != -1)
			{
				m_uniformLocationMap.insert({
					codeUniform.name, // key=Uniform name
					{ codeUniform.semantic, uniformId } // value=GlProgramUniform
				});
			}
			else
			{
				MIKAN_LOG_WARNING("GlProgram::CreateGLResources")
					<< m_code.getProgramName()
					<< " - Unable to find " << codeUniform.name << " uniform!";
			}
		}

		glUseProgram(m_programID);
		glUseProgram(0);

		return true;
	}

	return false;
}

void GlProgram::deleteProgram()
{
	if (m_programID != 0)
	{
		glDeleteProgram(m_programID);
		m_programID = 0;
	}
}

bool GlProgram::bindProgram() const
{
	if (m_programID != 0)
	{
		glUseProgram(m_programID);

		return true;
	}

	return false;
}

void GlProgram::unbindProgram() const
{
	if (m_programID != 0)
	{
		glUseProgram(0);
	}
}