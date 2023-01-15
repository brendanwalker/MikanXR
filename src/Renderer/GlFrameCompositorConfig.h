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

	eCompositorStencilMode stencilMode;
	bool bInvertWhenCameraInside;
	std::vector<MikanStencilID> quadStencils;
};

struct CompositorBoxStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode;
	std::vector<MikanStencilID> boxStencils;
};

struct CompositorModelStencilLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	eCompositorStencilMode stencilMode;
	std::vector<MikanStencilID> modelStencils;
};

struct CompositorLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	bool verticalFlip;
	eCompositorBlendMode blendMode;
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