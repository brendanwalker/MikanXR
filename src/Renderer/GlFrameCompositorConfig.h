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
	std::vector<std::string> quadStencilNames;
};

struct CompositorBoxStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode = eCompositorStencilMode::noStencil;
	std::vector<std::string> boxStencilNames;
};

struct CompositorModelStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode = eCompositorStencilMode::noStencil;
	std::vector<std::string> modelStencilNames;
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

class GlFrameCompositorConfig : public CommonConfig
{
public:
	GlFrameCompositorConfig(const std::string& fnamebase = "FrameCompositorConfig")
		: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string name;
	std::vector<CompositorLayerConfig> layers;
};