#include "GlFrameCompositorConfig.h"
#include "StringUtils.h"
#include "NodeGraphAssetReference.h"

// -- CompositorPreset ------
const std::string CompositorPreset::k_compositorGraphAssetRefPropertyId= "compositorGraphAssetRef";

CompositorPreset::CompositorPreset(const std::string& fnamebase)
	: CommonConfig(fnamebase)
	, compositorGraphAssetRefConfig(std::make_shared<AssetReferenceConfig>("CompositorGraph"))
{
}

configuru::Config CompositorPreset::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["name"] = name;
	pt["builtIn"]= builtIn;

	if (compositorGraphAssetRefConfig)
	{
		pt["compositor_graph"] = compositorGraphAssetRefConfig->writeToJSON();
	}

	return pt;
}

void CompositorPreset::readFromJSON(
	const configuru::Config& pt)
{
	CommonConfig::writeToJSON();

	name= pt.get_or<std::string>("name", name);
	builtIn= pt.get_or<bool>("builtIn", builtIn);

	compositorGraphAssetRefConfig= NodeGraphAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key("compositor_graph"))
	{
		compositorGraphAssetRefConfig->readFromJSON(pt["compositor_graph"]);
	}
}

// -- GlFrameCompositorConfig ------
const std::string GlFrameCompositorConfig::k_presetNamePropertyId= "presetName";

configuru::Config GlFrameCompositorConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["presetName"] = presetName;
	pt["nextPresetId"] = nextPresetId;

	return pt;
}

void GlFrameCompositorConfig::readFromJSON(
	const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	presetName = pt.get_or<std::string>("presetName", presetName);
	nextPresetId = pt.get_or<int>("nextPresetId", nextPresetId);
}