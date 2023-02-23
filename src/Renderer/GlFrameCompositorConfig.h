#pragma once

#include "FrameCompositorConstants.h"
#include "CommonConfig.h"

#include <filesystem>
#include <string>
#include <map>
#include <vector>

typedef int32_t MikanStencilID;

struct CompositorLayerShaderConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	std::string materialName;

	// uniform name -> frame compositor data source name
	std::map<std::string, std::string> floatSourceMap;
	std::map<std::string, std::string> float2SourceMap;
	std::map<std::string, std::string> float3SourceMap;
	std::map<std::string, std::string> float4SourceMap;
	std::map<std::string, std::string> mat4SourceMap;
	std::map<std::string, std::string> colorTextureSourceMap;
};

struct CompositorQuadStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode = eCompositorStencilMode::noStencil;
	bool bInvertWhenCameraInside = false;
	std::vector<int> quadStencilIds;
};

struct CompositorBoxStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode = eCompositorStencilMode::noStencil;
	std::vector<int> boxStencilIds;
};

struct CompositorModelStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode = eCompositorStencilMode::noStencil;
	std::vector<int> modelStencilIds;
};

struct CompositorLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	bool verticalFlip = false;
	eCompositorBlendMode blendMode= eCompositorBlendMode::blendOff;
	CompositorLayerShaderConfig shaderConfig;
	CompositorQuadStencilLayerConfig quadStencilConfig;
	CompositorBoxStencilLayerConfig boxStencilConfig;
	CompositorModelStencilLayerConfig modelStencilConfig;
};

class CompositorPreset : public CommonConfig
{
public:
	CompositorPreset(const std::string& fnamebase = "CompositorPreset")
		: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string name;
	bool builtIn= false;
	std::vector<CompositorLayerConfig> layers;
};

class GlFrameCompositorConfig : public CommonConfig
{
public:
	GlFrameCompositorConfig(const std::string& fnamebase = "FrameCompositorConfig")
		: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string presetName;
	int nextPresetId= 0;
};