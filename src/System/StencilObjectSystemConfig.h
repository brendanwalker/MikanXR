#pragma once

#include "CommonConfig.h"
#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
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

	static const std::string k_quadStencilListPropertyId;
	QuadStencilConfigConstPtr getQuadStencilConfigConst(MikanStencilID stencilId) const;
	QuadStencilConfigPtr getQuadStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);

	static const std::string k_boxStencilListPropertyId;
	BoxStencilConfigConstPtr getBoxStencilConfigConst(MikanStencilID stencilId) const;
	BoxStencilConfigPtr getBoxStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);

	static const std::string k_modelStencilListPropertyId;
	ModelStencilConfigConstPtr getModelStencilConfigConst(MikanStencilID stencilId) const;
	ModelStencilConfigPtr getModelStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);

	std::vector<QuadStencilConfigPtr> quadStencilList;
	std::vector<BoxStencilConfigPtr> boxStencilList;
	std::vector<ModelStencilConfigPtr> modelStencilList;
	MikanStencilID nextStencilId= 0;
	bool debugRenderStencils= false;
};