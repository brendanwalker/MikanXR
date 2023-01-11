#include "GlCommon.h"
#include "GlProgram.h"
#include "GlProgramConfig.h"
#include "GlTexture.h"
#include "Logger.h"
#include "StringUtils.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <assert.h>

const std::string g_UniformSemanticName[(int)eUniformSemantic::COUNT] = {
	"transformMatrix",
	"modelViewProjectionMatrix",
	"diffuseColorRGBA",
	"diffuseColorRGB",
	"screenPosition",
	"floatConstant0",
	"floatConstant1",
	"floatConstant2",
	"floatConstant3",
	"texture0",
	"texture1",
	"texture2",
	"texture3",
	"texture4",
	"texture5",
	"texture6",
	"texture7",
	"texture8",
	"texture9",
	"texture10",
	"texture11",
	"texture12",
	"texture13",
	"texture14",
	"texture15",
	"texture16",
	"texture17",
	"texture18",
	"texture19",
	"texture20",
	"texture21",
	"texture22",
	"texture23",
	"texture24",
	"texture25",
	"texture26",
	"texture27",
	"texture28",
	"texture29",
	"texture30",
	"texture31",
};
const std::string* k_UniformSemanticName = g_UniformSemanticName;

// -- GlProgramCode -----
GlProgramCode::GlProgramCode(
	const std::string& filename,
	const std::string& vertexCode, 
	const std::string& fragmentCode)
	: m_filename(filename)
	, m_vertexShaderCode(vertexCode)
	, m_framementShaderCode(fragmentCode)
{
	std::hash<std::string> hasher;

	m_shaderCodeHash= hasher(vertexCode + fragmentCode);
}

bool GlProgramCode::loadFromConfig(const GlProgramConfig* config)
{
	bool bSuccess= true;

	m_filename= config->getConfigPath();

	try
	{
		std::ifstream t(config->vertexShaderPath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		m_vertexShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("GlProgramCode::loadFromConfig")
			<< m_filename
			<< " - unable to load vertex shader file!";
		bSuccess= false;
	}

	try
	{
		std::ifstream t(config->fragmentShaderPath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		m_framementShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("GlProgramCode::loadFromConfig")
			<< m_filename
			<< " - unable to load fragment shader file!";
		bSuccess = false;
	}

	for (const auto& [name, semanticString] : config->uniforms)
	{
		eUniformSemantic semantic= 
			StringUtils::FindEnumValue<eUniformSemantic>(
				semanticString, g_UniformSemanticName);

		if (semantic != eUniformSemantic::INVALID)
		{
			m_uniformList.push_back({name, semantic});
		}
		else
		{
			MIKAN_LOG_ERROR("GlProgramCode::loadFromConfig")
				<< "Invalid semantic: "
				<< name
				<< " -> "
				<< semanticString;
			bSuccess = false;
		}
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

bool GlProgram::setMatrix4x4Uniform(
	const eUniformSemantic semantic,
	const glm::mat4& mat)
{
	assert(semantic == eUniformSemantic::transformMatrix || semantic == eUniformSemantic::modelViewProjectionMatrix);
	auto iter= m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(mat));
		return true;
	}
	return false;
}

bool GlProgram::setFloatUniform(
	const eUniformSemantic semantic,
	const float value)
{
	assert(semantic >= eUniformSemantic::floatConstant0 && semantic <= eUniformSemantic::floatConstant3);
	auto iter = m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform1f(uniformId, value);
		return true;
	}
	return false;
}

bool GlProgram::setVector2Uniform(
	const eUniformSemantic semantic, 
	const glm::vec2& vec)
{
	assert(semantic == eUniformSemantic::screenPosition);
	auto iter = m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform2fv(uniformId, 1, glm::value_ptr(vec));
		return true;
	}
	return false;
}

bool GlProgram::setVector3Uniform(
	const eUniformSemantic semantic, 
	const glm::vec3& vec)
{
	assert(semantic == eUniformSemantic::diffuseColorRGB);
	auto iter = m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform3fv(uniformId, 1, glm::value_ptr(vec));
		return true;
	}
	return false;
}

bool GlProgram::setVector4Uniform(
	const eUniformSemantic semantic,
	const glm::vec4& vec)
{
	assert(semantic == eUniformSemantic::diffuseColorRGBA);
	auto iter = m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		glUniform4fv(uniformId, 1, glm::value_ptr(vec));
		return true;
	}
	return false;
}

bool GlProgram::setTextureUniform(
	const eUniformSemantic semantic)
{
	assert(semantic >= eUniformSemantic::texture0 && semantic <= eUniformSemantic::texture31);
	auto iter = m_uniformLocationMap.find(semantic);
	if (iter != m_uniformLocationMap.end())
	{
		GLint uniformId = iter->second.locationId;
		GLint textureUnit = (GLint)semantic - (GLint)eUniformSemantic::texture0;

		glUniform1i(uniformId, textureUnit);
		return true;
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
		checkGLError(__FILE__, __LINE__);

		int vShaderCompiled = 0;
		glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
		checkGLError(__FILE__, __LINE__);

		if (vShaderCompiled != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::createProgram")
				<< m_code.getFilename()
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
		checkGLError(__FILE__, __LINE__);

		int fShaderCompiled = 0;
		glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		checkGLError(__FILE__, __LINE__);

		if (fShaderCompiled != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::CreateGLResources")
				<< m_code.getFilename()
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
		checkGLError(__FILE__, __LINE__);

		int programSuccess = 1;
		glGetProgramiv(m_programID, GL_LINK_STATUS, &programSuccess);
		checkGLError(__FILE__, __LINE__);

		if (programSuccess != 1)
		{
			MIKAN_LOG_ERROR("GlProgram::CreateGLResources")
				<< m_code.getFilename()
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
			checkGLError(__FILE__, __LINE__);

			if (uniformId != -1)
			{
				m_uniformLocationMap.insert({
					codeUniform.semantic, // key=eUniformSemantic
					{ codeUniform.name, codeUniform.semantic, uniformId } // value=GlProgram::Uniform
					});
			}
			else
			{
				MIKAN_LOG_WARNING("GlProgram::CreateGLResources")
					<< m_code.getFilename()
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