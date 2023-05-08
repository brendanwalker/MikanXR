#pragma once

#include "CommonConfig.h"
#include "MikanClientTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ProfileConfigConstants.h"

#include <filesystem>


class StencilObjectSystemConfig : public CommonConfig
{
public:
	StencilObjectSystemConfig(const std::string& fnamebase = "StencilObjectSystemConfig")
		: CommonConfig(fnamebase)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddStencil() const;
	bool removeStencil(MikanStencilID stencilId);
	eStencilType getStencilType(MikanStencilID stencilId) const;

	QuadStencilConfigConstPtr getQuadStencilInfoConst(MikanStencilID stencilId) const;
	QuadStencilConfigPtr getQuadStencilInfo(MikanStencilID stencilId);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);

	BoxStencilConfigConstPtr getBoxStencilInfoConst(MikanStencilID stencilId) const;
	BoxStencilConfigPtr getBoxStencilInfo(MikanStencilID stencilId);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);

	ModelStencilConfigConstPtr getModelStencilConfigConst(MikanStencilID stencilId) const;
	ModelStencilConfigPtr getModelStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);

	std::vector<QuadStencilConfigPtr> quadStencilList;
	std::vector<BoxStencilConfigPtr> boxStencilList;
	std::vector<ModelStencilConfigPtr> modelStencilList;
	MikanStencilID nextStencilId= 0;
	bool debugRenderStencils= false;
};