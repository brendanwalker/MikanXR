#pragma once

#include "FrameCompositorConstants.h"
#include "CommonConfig.h"
#include "CompositorFwd.h"
#include "AssetFwd.h"

#include <filesystem>
#include <string>
#include <memory>

typedef int32_t MikanStencilID;

class CompositorPreset : public CommonConfig
{
public:
	CompositorPreset(const std::string& fnamebase = "CompositorPreset");

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string name;
	bool builtIn= false;

	static const std::string k_compositorGraphAssetRefPropertyId;
	AssetReferenceConfigPtr compositorGraphAssetRefConfig;
};

class GlFrameCompositorConfig : public CommonConfig
{
public:
	GlFrameCompositorConfig(const std::string& fnamebase = "FrameCompositorConfig")
		: CommonConfig(fnamebase)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_presetNamePropertyId;
	std::string presetName;
	int nextPresetId= 0;
};