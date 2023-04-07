#pragma once

#include "CommonConfig.h"
#include "MikanClientTypes.h"
#include "ProfileConfigConstants.h"

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

	bool canAddStencil() const;
	bool removeStencil(MikanStencilID stencilId);
	eStencilType getStencilType(MikanStencilID stencilId) const;

	const MikanStencilQuad* getQuadStencilInfo(MikanStencilID stencilId) const;
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);

	const MikanStencilBox* getBoxStencilInfo(MikanStencilID stencilId) const;
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);

	const MikanStencilModelConfig* getModelStencilConfig(MikanStencilID stencilId) const;
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);

	std::vector<MikanStencilQuad> quadStencilList;
	std::vector<MikanStencilBox> boxStencilList;
	std::vector<MikanStencilModelConfig> modelStencilList;
	MikanStencilID nextStencilId;
	bool debugRenderStencils;
};