#pragma once

#include "CommonConfig.h"
#include "MikanClientTypes.h"

#include <filesystem>

struct MikanStencilModelConfig
{
	MikanStencilModel modelInfo;
	std::filesystem::path modelPath;
};

class StencilObjectSystemConfig : public CommonConfig
{
public:
	StencilObjectSystemConfig(const std::string& fnamebase = "StencilObjectSystemConfig")
		: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::vector<MikanStencilQuad> quadStencilList;
	std::vector<MikanStencilBox> boxStencilList;
	std::vector<MikanStencilModelConfig> modelStencilList;
	MikanStencilID nextStencilId;
	bool debugRenderStencils;
};
