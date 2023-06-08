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
	QuadStencilDefinitionConstPtr getQuadStencilConfigConst(MikanStencilID stencilId) const;
	QuadStencilDefinitionPtr getQuadStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);

	static const std::string k_boxStencilListPropertyId;
	BoxStencilDefinitionConstPtr getBoxStencilConfigConst(MikanStencilID stencilId) const;
	BoxStencilDefinitionPtr getBoxStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);

	static const std::string k_modelStencilListPropertyId;
	ModelStencilDefinitionConstPtr getModelStencilConfigConst(MikanStencilID stencilId) const;
	ModelStencilDefinitionPtr getModelStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);

	std::vector<QuadStencilDefinitionPtr> quadStencilList;
	std::vector<BoxStencilDefinitionPtr> boxStencilList;
	std::vector<ModelStencilDefinitionPtr> modelStencilList;
	MikanStencilID nextStencilId= 0;
	bool debugRenderStencils= false;
};