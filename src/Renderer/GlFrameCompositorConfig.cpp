#include "GlFrameCompositorConfig.h"
#include "StringUtils.h"

static eCompositorStencilMode parseCompositorStencilMode(const configuru::Config& pt)
{
	const std::string stencilModeString =
		pt.get_or<std::string>(
			"stencilMode",
			k_compositorStencilModeStrings[(int)eCompositorStencilMode::insideStencil]);

	return StringUtils::FindEnumValue<eCompositorStencilMode>(stencilModeString, k_compositorStencilModeStrings);
}

// -- CompositorLayerConfig ------
const configuru::Config CompositorLayerShaderConfig::writeToJSON() const
{
	configuru::Config pt{
		{"materialName", materialName},
	};

	if (floatSourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "floatSourceMap", floatSourceMap);
	if (float2SourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "float2SourceMap", float2SourceMap);
	if (float3SourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "float3SourceMap", float3SourceMap);
	if (float4SourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "float4SourceMap", float4SourceMap);
	if (mat4SourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "mat4SourceMap", mat4SourceMap);
	if (colorTextureSourceMap.size() > 0)
		CommonConfig::writeStdMap(pt, "textureSourceMap", colorTextureSourceMap);

	return pt;
}

void CompositorLayerShaderConfig::readFromJSON(const configuru::Config& pt)
{
	materialName = pt.get_or<std::string>("materialName", materialName);

	if (pt.has_key("floatSourceMap"))
		CommonConfig::readStdMap(pt, "floatSourceMap", floatSourceMap);
	if (pt.has_key("float2SourceMap"))
		CommonConfig::readStdMap(pt, "float2SourceMap", float2SourceMap);
	if (pt.has_key("float3SourceMap"))
		CommonConfig::readStdMap(pt, "float3SourceMap", float3SourceMap);
	if (pt.has_key("float4SourceMap"))
		CommonConfig::readStdMap(pt, "float4SourceMap", float4SourceMap);
	if (pt.has_key("mat4SourceMap"))
		CommonConfig::readStdMap(pt, "mat4SourceMap", mat4SourceMap);
	if (pt.has_key("textureSourceMap"))
		CommonConfig::readStdMap(pt, "textureSourceMap", colorTextureSourceMap);
}

// -- CompositorQuadStencilLayerConfig ------
const configuru::Config CompositorQuadStencilLayerConfig::writeToJSON() const
{
	configuru::Config pt{
		{"stencilMode", k_compositorStencilModeStrings[(int)stencilMode]},
		{"bInvertWhenCameraInside", bInvertWhenCameraInside},
	};

	if (quadStencilNames.size() > 0)
		CommonConfig::writeStdVector(pt, "quadStencilNames", quadStencilNames);

	return pt;
}

void CompositorQuadStencilLayerConfig::readFromJSON(const configuru::Config& pt)
{
	stencilMode = parseCompositorStencilMode(pt);
	bInvertWhenCameraInside = pt.get_or<bool>("bInvertWhenCameraInside", bInvertWhenCameraInside);

	if (pt.has_key("quadStencilNames"))
		CommonConfig::readStdVector(pt, "quadStencilNames", quadStencilNames);
}

// -- CompositorBoxStencilLayerConfig ------
const configuru::Config CompositorBoxStencilLayerConfig::writeToJSON() const
{
	configuru::Config pt{
		{"stencilMode", k_compositorStencilModeStrings[(int)stencilMode]},
	};

	if (boxStencilNames.size() > 0)
		CommonConfig::writeStdVector(pt, "boxStencilNames", boxStencilNames);

	return pt;
}

void CompositorBoxStencilLayerConfig::readFromJSON(const configuru::Config& pt)
{
	stencilMode = parseCompositorStencilMode(pt);

	if (pt.has_key("boxStencilNames"))
		CommonConfig::readStdVector(pt, "boxStencilNames", boxStencilNames);
}

// -- CompositorModelStencilLayerConfig ------
const configuru::Config CompositorModelStencilLayerConfig::writeToJSON() const
{
	configuru::Config pt{
		{"stencilMode", k_compositorStencilModeStrings[(int)stencilMode]},
	};

	if (modelStencilNames.size() > 0)
		CommonConfig::writeStdVector(pt, "modelStencilNames", modelStencilNames);

	return pt;
}

void CompositorModelStencilLayerConfig::readFromJSON(const configuru::Config& pt)
{
	stencilMode = parseCompositorStencilMode(pt);

	if (pt.has_key("modelStencilNames"))
		CommonConfig::readStdVector(pt, "modelStencilNames", modelStencilNames);
}

// -- CompositorLayerConfig -----
const configuru::Config CompositorLayerConfig::writeToJSON() const
{
	configuru::Config pt{
		{"verticalFlip", verticalFlip},
		{"blendMode", k_compositorBlendModeStrings[(int)blendMode]},
		{"shaderConfig", shaderConfig.writeToJSON()},
		{"quadStencilConfig", quadStencilConfig.writeToJSON()},
		{"boxStencilConfig", boxStencilConfig.writeToJSON()},
		{"modelStencilConfig", modelStencilConfig.writeToJSON()},
	};

	return pt;
}

void CompositorLayerConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string blendModeString =
		pt.get_or<std::string>(
			"blendMode",
			k_compositorBlendModeStrings[(int)eCompositorBlendMode::blendOff]);
	blendMode= StringUtils::FindEnumValue<eCompositorBlendMode>(blendModeString, k_compositorBlendModeStrings);

	verticalFlip= pt.get_or<bool>("verticalFlip", verticalFlip);

	shaderConfig.readFromJSON(pt["shaderConfig"]);
	
	if (pt.has_key("quadStencilConfig"))
	{
		quadStencilConfig.readFromJSON(pt["quadStencilConfig"]);
	}
	else
	{
		quadStencilConfig.bInvertWhenCameraInside= false;
		quadStencilConfig.quadStencilNames.clear();
		quadStencilConfig.stencilMode = eCompositorStencilMode::insideStencil;
	}

	if (pt.has_key("boxStencilConfig"))
	{
		boxStencilConfig.readFromJSON(pt["boxStencilConfig"]);
	}
	else
	{
		boxStencilConfig.boxStencilNames.clear();
		boxStencilConfig.stencilMode = eCompositorStencilMode::insideStencil;
	}

	if (pt.has_key("modelStencilConfig"))
	{
		modelStencilConfig.readFromJSON(pt["modelStencilConfig"]);
	}
	else
	{
		modelStencilConfig.modelStencilNames.clear();
		modelStencilConfig.stencilMode = eCompositorStencilMode::insideStencil;
	}
}

// -- GlFrameCompositorConfig ------
const configuru::Config GlFrameCompositorConfig::writeToJSON()
{
	configuru::Config pt = configuru::Config::object();

	// Write out the layers
	std::vector<configuru::Config> layerConfigs;
	for (const CompositorLayerConfig& layer : layers)
	{
		layerConfigs.push_back(layer.writeToJSON());
	}
	pt.insert_or_assign(std::string("layers"), layerConfigs);
	pt["name"] = name;

	return pt;
}

void GlFrameCompositorConfig::readFromJSON(
	const configuru::Config& pt)
{
	name= pt.get_or<std::string>("name", name);

	layers.clear();
	if (pt.has_key("layers"))
	{
		for (const configuru::Config& layerConfig : pt["layers"].as_array())
		{
			CompositorLayerConfig layer;

			layer.readFromJSON(layerConfig);
			layers.push_back(layer);
		}
	}
}