#include "IMkShaderCode.h"
#include "IMkVertexDefinition.h"

class GlShaderCode : public IMkShaderCode
{
public:
	GlShaderCode() = default;

	GlShaderCode(const std::string& programName)
		: m_programName(programName)
		, m_shaderCodeHash(0)
	{}

	GlShaderCode(
		const std::string& programName,
		const std::string& vertexCode,
		const std::string& fragmentCode)
		: m_programName(programName)
		, m_vertexShaderCode(vertexCode)
		, m_fragmentShaderCode(fragmentCode)
	{
		std::hash<std::string> hasher;

		m_shaderCodeHash = hasher(vertexCode + fragmentCode);
	}

	virtual const std::string& getProgramName() const override 
	{ 
		return m_programName; 
	}

	virtual void setProgramName(const std::string& inName) override 
	{ 
		m_programName = inName;
	}

	virtual const char* getVertexShaderCode() const override
	{
		return m_vertexShaderCode.c_str();
	}

	virtual const std::filesystem::path& getVertexShaderFilePath() const override 
	{
		return m_vertexShaderFilePath;
	}

	virtual void setVertexShaderFilePath(const std::filesystem::path& path) override
	{
		m_vertexShaderFilePath= path;
	}

	virtual const char* getFragmentShaderCode() const override
	{
		return m_fragmentShaderCode.c_str();
	}

	virtual const std::filesystem::path& getFragmentShaderFilePath() const override 
	{
		return m_fragmentShaderFilePath;
	}

	virtual void setFragmentShaderFilePath(const std::filesystem::path& path) override
	{
		m_fragmentShaderFilePath= path;
	}

	virtual size_t getCodeHash() const override 
	{
		return m_shaderCodeHash;
	}

	virtual const std::vector<IMkVertexAttributeConstPtr>& getVertexAttributes() const override
	{ 
		return m_vertexAttributes; 
	}

	virtual IMkShaderCode& addVertexAttribute(
		const std::string& name,
		eVertexDataType dataType,
		eVertexSemantic semantic,
		bool isNormalized) override
	{
		m_vertexAttributes.push_back(createMkVertexAttribute(name, dataType, semantic, isNormalized));
		return *this;
	}

	virtual const std::vector<Uniform>& getUniformList() const override
	{ 
		return m_uniformList;
	}

	virtual IMkShaderCode& addUniform(const std::string& name, eUniformSemantic semantic) override
	{
		m_uniformList.push_back({name, semantic});
		return *this;
	}

	virtual bool hasCode() const override
	{
		return m_vertexShaderCode.size() > 0 && m_fragmentShaderCode.size() > 0;
	}

	virtual bool operator == (const IMkShaderCode& other) const override
	{
		return m_shaderCodeHash == other.getCodeHash();
	}

	virtual bool operator != (const IMkShaderCode& other) const override
	{
		return m_shaderCodeHash != other.getCodeHash();
	}

protected:
	std::string m_programName;
	std::string m_vertexShaderCode;
	std::filesystem::path m_vertexShaderFilePath;
	std::string m_fragmentShaderCode;
	std::filesystem::path m_fragmentShaderFilePath;
	std::vector<IMkVertexAttributeConstPtr> m_vertexAttributes;
	std::vector<Uniform> m_uniformList;
	size_t m_shaderCodeHash;
};

IMkShaderCodePtr createIMkShaderCode()
{
	return std::make_shared<GlShaderCode>();
}

IMkShaderCodePtr createIMkShaderCode(const std::string& programName)
{
	return std::make_shared<GlShaderCode>(programName);
}

IMkShaderCodePtr createIMkShaderCode(
	const std::string& programName,
	const std::string& vertexCode,
	const std::string& fragmentCode)
{
	return std::make_shared<GlShaderCode>(programName, vertexCode, fragmentCode);
}