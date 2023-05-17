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

	QuadStencilConfigConstPtr getQuadStencilConfigConst(MikanStencilID stencilId) const;
	QuadStencilConfigPtr getQuadStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewQuadStencil(const MikanStencilQuad& quad);
	MulticastDelegate<void()> OnQuadStencilListChanged;
	MulticastDelegate<void(MikanStencilID stencilId)> OnQuadStencilModified;

	BoxStencilConfigConstPtr getBoxStencilConfigConst(MikanStencilID stencilId) const;
	BoxStencilConfigPtr getBoxStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewBoxStencil(const MikanStencilBox& quad);
	MulticastDelegate<void()> OnBoxStencilListChanged;
	MulticastDelegate<void(MikanStencilID stencilId)> OnBoxStencilModified;

	ModelStencilConfigConstPtr getModelStencilConfigConst(MikanStencilID stencilId) const;
	ModelStencilConfigPtr getModelStencilConfig(MikanStencilID stencilId);
	MikanStencilID addNewModelStencil(const MikanStencilModel& model);
	MulticastDelegate<void()> OnModelStencilListChanged;
	MulticastDelegate<void(MikanStencilID stencilId)> OnModelStencilModified;

	std::vector<QuadStencilConfigPtr> quadStencilList;
	std::vector<BoxStencilConfigPtr> boxStencilList;
	std::vector<ModelStencilConfigPtr> modelStencilList;
	MikanStencilID nextStencilId= 0;
	bool debugRenderStencils= false;
};