#include "GlCommon.h"
#include "IMkShaderCode.h"
#include "IMkShader.h"
#include "MkShaderConstants.h"
#include "IMkTexture.h"
#include "Logger.h"

#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <assert.h>

class GlShader : public IMkShader
{
public:

	GlShader() = default;
	GlShader(const std::string& programName)
		: m_code(createIMkShaderCode(programName))
	{}

	GlShader(IMkShaderCodeConstPtr code)
		: m_code(code)
	{}

	~GlShader()
	{
		deleteProgram();
	}

	virtual IMkVertexDefinitionConstPtr getVertexDefinition() const override
	{ 
		return m_vertexDefinition;
	}

	virtual IMkShaderCodeConstPtr getProgramCode() const override
	{
		return m_code;
	}

	virtual IMkShaderCodePtr getProgramCodeMutable() override
	{
		return std::const_pointer_cast<IMkShaderCode>(m_code);
	}

	virtual bool getUniformSemantic(
		const std::string uniformName,
		eUniformSemantic& outSemantic) const override
	{
		auto it = m_uniformLocationMap.find(uniformName);
		if (it != m_uniformLocationMap.end())
		{
			outSemantic = it->second.semantic;
			return true;
		}

		return false;
	}

	virtual bool getUniformDataType(
		const std::string uniformName, 
		eUniformDataType& outDataType) const override
	{
		eUniformSemantic semantic = eUniformSemantic::INVALID;
		if (getUniformSemantic(uniformName, semantic))
		{
			outDataType = getUniformSemanticDataType(semantic);
			return true;
		}

		return false;
	}

	virtual std::vector<std::string> getUniformNamesOfDataType(
		const eUniformDataType dataType) const override
	{
		std::vector<std::string> uniformNames;

		for (auto it = m_uniformLocationMap.begin(); it != m_uniformLocationMap.end(); it++)
		{
			const eUniformSemantic uniformSemantic = it->second.semantic;
			const eUniformDataType uniformDataType = getUniformSemanticDataType(uniformSemantic);

			if (uniformDataType == dataType)
			{
				uniformNames.push_back(it->first);
			}
		}

		return uniformNames;
	}

	virtual bool getFirstUniformNameOfSemantic(
		eUniformSemantic semantic, 
		std::string& outUniformName) const override
	{
		for (auto it = getUniformBegin(); it != getUniformEnd(); ++it)
		{
			if (it->second.semantic == semantic)
			{
				outUniformName = it->first;
				return true;
			}
		}

		return false;
	}

	virtual MkShaderUniformIter getUniformBegin() const override
	{ 
		return m_uniformLocationMap.begin(); 
	}
	
	virtual MkShaderUniformIter getUniformEnd() const override
	{ 
		return m_uniformLocationMap.end(); 
	}

	virtual bool getFirstTextureUnitOfSemantic(
		eUniformSemantic semantic,
		int& outTextureUnit) const override
	{
		std::string uniformName;

		return getFirstUniformNameOfSemantic(semantic, uniformName) &&
			getUniformTextureUnit(uniformName, outTextureUnit);
	}

	virtual bool getUniformTextureUnit(
		const std::string uniformName,
		int& outTextureUnit) const override
	{
		auto it = m_textureUnitMap.find(uniformName);
		if (it != m_textureUnitMap.end())
		{
			outTextureUnit = it->second;
			return true;
		}

		return false;
	}

	virtual bool setMatrix4x4Uniform(
		const std::string uniformName,
		const glm::mat4& mat) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(mat));
			return !checkHasAnyMkError("IMkShader::setMatrix4x4Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setIntUniform(
		const std::string uniformName,
		const int value) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform1i(uniformId, value);
			return !checkHasAnyMkError("IMkShader::setIntUniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setInt2Uniform(
		const std::string uniformName,
		const glm::ivec2& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform2iv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setInt2Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setInt3Uniform(
		const std::string uniformName,
		const glm::ivec3& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform3iv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setInt3Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setInt4Uniform(
		const std::string uniformName,
		const glm::ivec4& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform4iv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setInt3Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setFloatUniform(
		const std::string uniformName,
		const float value) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform1f(uniformId, value);
			return !checkHasAnyMkError("IMkShader::setFloatUniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setVector2Uniform(
		const std::string uniformName,
		const glm::vec2& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform2fv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setVector2Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setVector3Uniform(
		const std::string uniformName,
		const glm::vec3& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform3fv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setVector3Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setVector4Uniform(
		const std::string uniformName,
		const glm::vec4& vec) override
	{
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end())
		{
			GLint uniformId = iter->second.locationId;
			glUniform4fv(uniformId, 1, glm::value_ptr(vec));
			return !checkHasAnyMkError("IMkShader::setVector4Uniform()", __FILE__, __LINE__);
		}
		return false;
	}

	virtual bool setTextureUniform(
		const std::string uniformName) override
	{
		GLint textureUnit = 0;
		auto iter = m_uniformLocationMap.find(uniformName);
		if (iter != m_uniformLocationMap.end() &&
			getUniformTextureUnit(uniformName, textureUnit))
		{
			const GLint uniformId = iter->second.locationId;

			glUniform1i(uniformId, textureUnit);
			return !checkHasAnyMkError("IMkShader::setTextureUniform()", __FILE__, __LINE__);
		}

		return false;
	}

	virtual bool compileProgram() override
	{
		const std::string& programName = m_code->getProgramName();

		// Nuke any existing program
		deleteProgram();

		if (m_code->hasCode())
		{
			m_programID = glCreateProgram();
			if (m_programID == 0)
			{
				MIKAN_LOG_ERROR("IMkShader::createProgram") << "glCreateProgram failed";
				return false;
			}

			if (!programName.empty())
			{
				glObjectLabel(GL_PROGRAM, m_programID, -1, programName.c_str());
			}

			uint32_t nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
			if (nSceneVertexShader == 0)
			{
				checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);
				return false;
			}
			if (!programName.empty())
			{
				glObjectLabel(GL_SHADER, nSceneVertexShader, -1, programName.c_str());
			}

			const GLchar* vertexShaderSource = (const GLchar*)m_code->getVertexShaderCode();
			glShaderSource(nSceneVertexShader, 1, &vertexShaderSource, nullptr);
			if (checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__))
			{
				return false;
			}

			glCompileShader(nSceneVertexShader);
			if (checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__))
			{
				return false;
			}

			int vShaderCompiled = 0;
			glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
			if (checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__))
			{
				return false;
			}

			if (vShaderCompiled != 1)
			{
				MIKAN_LOG_ERROR("IMkShader::createProgram")
					<< m_code->getProgramName()
					<< " - Unable to compile vertex shader "
					<< nSceneVertexShader;

				GLchar strInfoLog[1024] = {0};
				glGetShaderInfoLog(nSceneVertexShader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
				MIKAN_LOG_ERROR("IMkShader::createProgram") << strInfoLog;

				glDeleteProgram(m_programID);
				glDeleteShader(nSceneVertexShader);
				m_programID = 0;

				return false;
			}
			glAttachShader(m_programID, nSceneVertexShader);
			glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

			uint32_t nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			const GLchar* fragmentShaderSource = (const GLchar*)m_code->getFragmentShaderCode();
			glShaderSource(nSceneFragmentShader, 1, &fragmentShaderSource, nullptr);
			glCompileShader(nSceneFragmentShader);
			checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);

			if (!programName.empty())
			{
				glObjectLabel(GL_SHADER, nSceneFragmentShader, -1, programName.c_str());
			}

			int fShaderCompiled = 0;
			glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
			checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);

			if (fShaderCompiled != 1)
			{
				MIKAN_LOG_ERROR("IMkShader::CreateGLResources")
					<< m_code->getProgramName()
					<< " - Unable to compile fragment shader "
					<< nSceneFragmentShader;

				GLchar strInfoLog[1024] = {0};
				glGetShaderInfoLog(nSceneFragmentShader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
				MIKAN_LOG_ERROR("IMkShader::createProgram") << strInfoLog;

				glDeleteProgram(m_programID);
				glDeleteShader(nSceneFragmentShader);
				m_programID = 0;

				return false;
			}
			glAttachShader(m_programID, nSceneFragmentShader);
			glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

			glLinkProgram(m_programID);
			checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);

			int programSuccess = 1;
			glGetProgramiv(m_programID, GL_LINK_STATUS, &programSuccess);
			checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);

			if (programSuccess != 1)
			{
				MIKAN_LOG_ERROR("IMkShader::CreateGLResources")
					<< m_code->getProgramName()
					<< " - Error linking program "
					<< m_programID;

				GLchar strInfoLog[1024] = {0};
				glGetProgramInfoLog(m_programID, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
				MIKAN_LOG_ERROR("IMkShader::createProgram") << strInfoLog;

				glDeleteProgram(m_programID);
				m_programID = 0;

				return false;
			}

			// Create the uniform and texture unit map
			for (const IMkShaderCode::Uniform& codeUniform : m_code->getUniformList())
			{
				GLint uniformId = glGetUniformLocation(m_programID, codeUniform.name.c_str());
				checkHasAnyMkError("IMkShader::createProgram()", __FILE__, __LINE__);

				if (uniformId != -1)
				{
					eUniformDataType dataType = getUniformSemanticDataType(codeUniform.semantic);

					m_uniformLocationMap.insert({
						codeUniform.name, // key=Uniform name
						{ codeUniform.semantic, uniformId } // value=IMkShaderUniform
												});

					// Assign texture units in order the uniforms were specified
					if (dataType == eUniformDataType::datatype_texture)
					{
						GLint textureUnit = (GLint)m_textureUnitMap.size();
						m_textureUnitMap.insert({codeUniform.name, textureUnit});
					}
				}
				else
				{
					MIKAN_LOG_WARNING("IMkShader::compileProgram")
						<< m_code->getProgramName()
						<< " - Unable to find " << codeUniform.name << " uniform!";
				}
			}

			glUseProgram(m_programID);
			glUseProgram(0);

			// Create the vertex definition from the vertex attributes set on the program code
			m_vertexDefinition = createMkVertexDefinition(m_code->getVertexAttributes());

			// Last step: check that the vertex definition is compatible with the program
			return m_vertexDefinition->isCompatibleProgram(*this);
		}

		return false;
	}

	virtual bool isProgramCompiled() const override
	{
		return m_programID != 0; 
	}

	virtual uint32_t getIMkShaderId() const override
	{ 
		return m_programID;
	}

	virtual void deleteProgram() override
	{
		if (m_programID != 0)
		{
			glDeleteProgram(m_programID);
			m_programID = 0;
		}
	}

	virtual bool bindProgram() const override
	{
		if (m_programID != 0)
		{
			glUseProgram(m_programID);

			return !checkHasAnyMkError("IMkShader::bindProgram()", __FILE__, __LINE__);;
		}

		return false;
	}

	virtual void unbindProgram() const override
	{
		if (m_programID != 0)
		{
			glUseProgram(0);
		}
	}

protected:
	IMkShaderCodeConstPtr m_code;
	uint32_t m_programID = 0;
	MkShaderUniformMap m_uniformLocationMap;
	MkUniformNameTextureUnitMap m_textureUnitMap;
	IMkVertexDefinitionPtr m_vertexDefinition;
};

IMkShaderPtr createIMkShader()
{
	return std::make_shared<GlShader>();
}

IMkShaderPtr createIMkShader(const std::string& programName)
{
	return std::make_shared<GlShader>(programName);
}

IMkShaderPtr createIMkShader(IMkShaderCodeConstPtr shaderCode)
{
	return std::make_shared<GlShader>(shaderCode);
}