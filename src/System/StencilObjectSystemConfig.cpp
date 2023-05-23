#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"
#include "StencilObjectSystemConfig.h"
#include "QuadStencilComponent.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- StencilObjectSystemConfig -----
const std::string StencilObjectSystemConfig::k_quadStencilListPropertyId= "quad_stencils";
const std::string StencilObjectSystemConfig::k_boxStencilListPropertyId= "box_stencils";
const std::string StencilObjectSystemConfig::k_modelStencilListPropertyId= "model_stencils";

configuru::Config StencilObjectSystemConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	// Stencils
	pt["nextStencilId"]= nextStencilId;
	pt["debugRenderStencils"]= debugRenderStencils;

	// Write out the quad stencils
	std::vector<configuru::Config> stencilQuadConfigs;
	for (QuadStencilConfigPtr stencil : quadStencilList)
	{
		stencilQuadConfigs.push_back(stencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("quadStencils"), stencilQuadConfigs);

	// Write out the box stencils
	std::vector<configuru::Config> stencilBoxConfigs;
	for (BoxStencilConfigPtr stencil : boxStencilList)
	{
		stencilBoxConfigs.push_back(stencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("boxStencils"), stencilBoxConfigs);

	// Write out the model stencils
	std::vector<configuru::Config> stencilModelConfigs;
	for (ModelStencilConfigPtr stencil : modelStencilList)
	{
		stencilModelConfigs.push_back(stencil->writeToJSON());
	}
	pt.insert_or_assign(std::string("modelStencils"), stencilModelConfigs);

	return pt;
}

void StencilObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	nextStencilId = pt.get_or<int>("nextStencilId", nextStencilId);
	debugRenderStencils = pt.get_or<bool>("debugRenderStencils", debugRenderStencils);

	// Read in the quad stencils
	quadStencilList.clear();
	if (pt.has_key("quadStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["quadStencils"].as_array())
		{			
			QuadStencilConfigPtr configPtr = std::make_shared<QuadStencilConfig>();
			configPtr->readFromJSON(stencilConfig);

			quadStencilList.push_back(configPtr);
			addChildConfig(configPtr);
		}
	}

	// Read in the box stencils
	boxStencilList.clear();
	if (pt.has_key("boxStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["boxStencils"].as_array())
		{
			BoxStencilConfigPtr configPtr = std::make_shared<BoxStencilConfig>();
			configPtr->readFromJSON(stencilConfig);

			boxStencilList.push_back(configPtr);
			addChildConfig(configPtr);
		}
	}

	// Read in the model stencils
	modelStencilList.clear();
	if (pt.has_key("modelStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["modelStencils"].as_array())
		{
			ModelStencilConfigPtr configPtr = std::make_shared<ModelStencilConfig>();
			configPtr->readFromJSON(stencilConfig);

			modelStencilList.push_back(configPtr);
			addChildConfig(configPtr);
		}
	}
}

bool StencilObjectSystemConfig::canAddStencil() const
{
	return (boxStencilList.size() + quadStencilList.size() + modelStencilList.size() < MAX_MIKAN_STENCILS);
}

bool StencilObjectSystemConfig::removeStencil(MikanStencilID stencilId)
{
	// Try quad stencil list first
	{
		auto it = std::find_if(
			quadStencilList.begin(), quadStencilList.end(),
			[stencilId](QuadStencilConfigPtr q) {
			return q->getStencilId() == stencilId;
		});

		if (it != quadStencilList.end())
		{
			removeChildConfig(*it);

			quadStencilList.erase(it);
			markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilListPropertyId));

			return true;
		}
	}

	// Then try the box stencil list
	{
		auto it = std::find_if(
			boxStencilList.begin(), boxStencilList.end(),
			[stencilId](BoxStencilConfigPtr b) {
			return b->getStencilId() == stencilId;
		});

		if (it != boxStencilList.end())
		{
			removeChildConfig(*it);

			boxStencilList.erase(it);
			markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilListPropertyId));

			return true;
		}
	}

	// Then try model stencil list last
	{
		auto it = std::find_if(
			modelStencilList.begin(), modelStencilList.end(),
			[stencilId](ModelStencilConfigPtr m) {
			return m->getStencilId() == stencilId;
		});

		if (it != modelStencilList.end())
		{
			removeChildConfig(*it);

			modelStencilList.erase(it);
			markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilListPropertyId));

			return true;
		}
	}

	return false;
}

eStencilType StencilObjectSystemConfig::getStencilType(MikanStencilID stencilId) const
{
	// Try quad stencil list first
	{
		auto it = std::find_if(
			quadStencilList.begin(), quadStencilList.end(),
			[stencilId](QuadStencilConfigPtr q) {
			return q->getStencilId() == stencilId;
		});

		if (it != quadStencilList.end())
		{
			return eStencilType::quad;
		}
	}

	// Then try the box stencil list
	{
		auto it = std::find_if(
			boxStencilList.begin(), boxStencilList.end(),
			[stencilId](BoxStencilConfigPtr b) {
			return b->getStencilId() == stencilId;
		});

		if (it != boxStencilList.end())
		{
			return eStencilType::box;
		}
	}

	// Then try model stencil list last
	{
		auto it = std::find_if(
			modelStencilList.begin(), modelStencilList.end(),
			[stencilId](ModelStencilConfigPtr m) {
			return m->getStencilId() == stencilId;
		});

		if (it != modelStencilList.end())
		{
			return eStencilType::model;
		}
	}


	return eStencilType::INVALID;
}

QuadStencilConfigConstPtr StencilObjectSystemConfig::getQuadStencilConfigConst(MikanStencilID stencilId) const
{
	auto it = std::find_if(
		quadStencilList.begin(), quadStencilList.end(),
		[stencilId](QuadStencilConfigPtr quadConfig) {
		return quadConfig->getStencilId() == stencilId;
	});

	if (it != quadStencilList.end())
	{
		return *it;
	}

	return nullptr;
}

QuadStencilConfigPtr StencilObjectSystemConfig::getQuadStencilConfig(MikanStencilID stencilId)
{
	return std::const_pointer_cast<QuadStencilConfig>(getQuadStencilConfigConst(stencilId));
}

MikanStencilID StencilObjectSystemConfig::addNewQuadStencil(const MikanStencilQuad& quadInfo)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilQuad localQuadInfo= quadInfo;
	localQuadInfo.stencil_id= nextStencilId;
	nextStencilId++;

	QuadStencilConfigPtr configPtr = std::make_shared<QuadStencilConfig>(localQuadInfo);
	addChildConfig(configPtr);

	quadStencilList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_quadStencilListPropertyId));

	return configPtr->getStencilId();
}

BoxStencilConfigConstPtr StencilObjectSystemConfig::getBoxStencilConfigConst(MikanStencilID stencilId) const
{
	auto it = std::find_if(
		boxStencilList.begin(), boxStencilList.end(),
		[stencilId](BoxStencilConfigPtr box) {
		return box->getStencilId() == stencilId;
	});

	if (it != boxStencilList.end())
	{
		return *it;
	}

	return nullptr;
}

BoxStencilConfigPtr StencilObjectSystemConfig::getBoxStencilConfig(MikanStencilID stencilId)
{
	return std::const_pointer_cast<BoxStencilConfig>(getBoxStencilConfigConst(stencilId));
}

MikanStencilID StencilObjectSystemConfig::addNewBoxStencil(const MikanStencilBox& boxInfo)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilBox localBoxInfo = boxInfo;
	localBoxInfo.stencil_id = nextStencilId;
	nextStencilId++;

	BoxStencilConfigPtr configPtr = std::make_shared<BoxStencilConfig>(localBoxInfo);
	addChildConfig(configPtr);

	boxStencilList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_boxStencilListPropertyId));

	return configPtr->getStencilId();
}

ModelStencilConfigConstPtr StencilObjectSystemConfig::getModelStencilConfigConst(MikanStencilID stencilId) const
{
	auto it = std::find_if(
		modelStencilList.begin(),
		modelStencilList.end(),
		[stencilId](ModelStencilConfigPtr stencil) {
		return stencil->getStencilId() == stencilId;
	});

	if (it != modelStencilList.end())
	{
		return *it;
	}
	else
	{
		return nullptr;
	}
}

ModelStencilConfigPtr StencilObjectSystemConfig::getModelStencilConfig(MikanStencilID stencilId)
{
	return std::const_pointer_cast<ModelStencilConfig>(getModelStencilConfigConst(stencilId));
}

MikanStencilID StencilObjectSystemConfig::addNewModelStencil(const MikanStencilModel& modelInfo)
{
	if (!canAddStencil())
		return INVALID_MIKAN_ID;

	MikanStencilModel localModelInfo = modelInfo;
	localModelInfo.stencil_id = nextStencilId;
	nextStencilId++;

	ModelStencilConfigPtr configPtr = std::make_shared<ModelStencilConfig>(localModelInfo);
	addChildConfig(configPtr);

	modelStencilList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_modelStencilListPropertyId));

	return configPtr->getStencilId();
}