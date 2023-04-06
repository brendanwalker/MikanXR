#include "StencilObjectSystemConfig.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

const configuru::Config StencilObjectSystemConfig::writeToJSON()
{
	configuru::Config pt{
		// Stencils
		{"nextStencilId", nextStencilId},
		{"debugRenderStencils", debugRenderStencils},
	};

	// Write out the quad stencils
	std::vector<configuru::Config> stencilQuadConfigs;
	for (const MikanStencilQuad& stencil : quadStencilList)
	{
		configuru::Config stencilConfig{
			{"stencil_id", stencil.stencil_id},
			{"parent_anchor_id", stencil.parent_anchor_id},
			{"quad_width", stencil.quad_width},
			{"quad_height", stencil.quad_height},
			{"is_double_sided", stencil.is_double_sided},
			{"is_disabled", stencil.is_disabled},
			{"stencil_name", stencil.stencil_name}
		};

		writeVector3f(stencilConfig, "quad_center", stencil.quad_center);
		writeVector3f(stencilConfig, "quad_x_axis", stencil.quad_x_axis);
		writeVector3f(stencilConfig, "quad_y_axis", stencil.quad_y_axis);
		writeVector3f(stencilConfig, "quad_normal", stencil.quad_normal);

		stencilQuadConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("quadStencils"), stencilQuadConfigs);

	// Write out the box stencils
	std::vector<configuru::Config> stencilBoxConfigs;
	for (const MikanStencilBox& stencil : boxStencilList)
	{
		configuru::Config stencilConfig{
			{"stencil_id", stencil.stencil_id},
			{"parent_anchor_id", stencil.parent_anchor_id},
			{"box_x_size", stencil.box_x_size},
			{"box_y_size", stencil.box_y_size},
			{"box_z_size", stencil.box_z_size},
			{"is_disabled", stencil.is_disabled},
			{"stencil_name", stencil.stencil_name}
		};

		writeVector3f(stencilConfig, "box_center", stencil.box_center);
		writeVector3f(stencilConfig, "box_x_axis", stencil.box_x_axis);
		writeVector3f(stencilConfig, "box_y_axis", stencil.box_y_axis);
		writeVector3f(stencilConfig, "box_z_axis", stencil.box_z_axis);

		stencilBoxConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("boxStencils"), stencilBoxConfigs);

	// Write out the model stencils
	std::vector<configuru::Config> stencilModelConfigs;
	for (const MikanStencilModelConfig& stencil : modelStencilList)
	{
		configuru::Config stencilConfig{
			{"model_path", stencil.modelPath.string()},
			{"stencil_id", stencil.modelInfo.stencil_id},
			{"parent_anchor_id", stencil.modelInfo.parent_anchor_id},
			{"is_disabled", stencil.modelInfo.is_disabled},
			{"stencil_name", stencil.modelInfo.stencil_name}
		};

		writeVector3f(stencilConfig, "model_position", stencil.modelInfo.model_position);
		writeRotator3f(stencilConfig, "model_rotator", stencil.modelInfo.model_rotator);
		writeVector3f(stencilConfig, "model_scale", stencil.modelInfo.model_scale);

		stencilModelConfigs.push_back(stencilConfig);
	}
	pt.insert_or_assign(std::string("modelStencils"), stencilModelConfigs);

	return pt;
}

void StencilObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	nextStencilId = pt.get_or<int>("nextStencilId", nextStencilId);
	debugRenderStencils = pt.get_or<bool>("debugRenderStencils", debugRenderStencils);

	// Read in the quad stencils
	quadStencilList.clear();
	if (pt.has_key("quadStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["quadStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilQuad stencil;
				memset(&stencil, 0, sizeof(stencil));

				stencil.stencil_id = stencilConfig.get<int>("stencil_id");
				stencil.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "quad_center", stencil.quad_center);
				readVector3f(stencilConfig, "quad_x_axis", stencil.quad_x_axis);
				readVector3f(stencilConfig, "quad_y_axis", stencil.quad_y_axis);
				readVector3f(stencilConfig, "quad_normal", stencil.quad_normal);
				stencil.quad_width = stencilConfig.get_or<float>("quad_width", 0.25f);
				stencil.quad_height = stencilConfig.get_or<float>("quad_height", 0.25f);
				stencil.is_double_sided = stencilConfig.get_or<bool>("is_double_sided", false);
				stencil.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name = stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());

				quadStencilList.push_back(stencil);
			}
		}
	}

	// Read in the box stencils
	boxStencilList.clear();
	if (pt.has_key("boxStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["boxStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilBox stencil;
				memset(&stencil, 0, sizeof(stencil));

				stencil.stencil_id = stencilConfig.get<int>("stencil_id");
				stencil.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "box_center", stencil.box_center);
				readVector3f(stencilConfig, "box_x_axis", stencil.box_x_axis);
				readVector3f(stencilConfig, "box_y_axis", stencil.box_y_axis);
				readVector3f(stencilConfig, "box_z_axis", stencil.box_z_axis);
				stencil.box_x_size = stencilConfig.get_or<float>("box_x_size", 0.25f);
				stencil.box_y_size = stencilConfig.get_or<float>("box_y_size", 0.25f);
				stencil.box_z_size = stencilConfig.get_or<float>("box_z_size", 0.25f);
				stencil.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name = stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(stencil.stencil_name, sizeof(stencil.stencil_name), "%s", stencil_name.c_str());

				boxStencilList.push_back(stencil);
			}
		}
	}

	// Read in the model stencils
	modelStencilList.clear();
	if (pt.has_key("modelStencils"))
	{
		for (const configuru::Config& stencilConfig : pt["modelStencils"].as_array())
		{
			if (stencilConfig.has_key("stencil_id"))
			{
				MikanStencilModelConfig modelConfig;
				MikanStencilModel& modelInfo = modelConfig.modelInfo;
				memset(&modelInfo, 0, sizeof(MikanStencilModel));

				modelConfig.modelPath = stencilConfig.get<std::string>("model_path");
				modelInfo.stencil_id = stencilConfig.get<int>("stencil_id");
				modelInfo.parent_anchor_id = stencilConfig.get_or<int>("parent_anchor_id", -1);
				readVector3f(stencilConfig, "model_position", modelInfo.model_position);
				readRotator3f(stencilConfig, "model_rotator", modelInfo.model_rotator);
				readVector3f(stencilConfig, "model_scale", modelInfo.model_scale);
				modelInfo.is_disabled = stencilConfig.get_or<bool>("is_disabled", false);

				const std::string stencil_name = stencilConfig.get_or<std::string>("stencil_name", "");
				StringUtils::formatString(modelInfo.stencil_name, sizeof(modelInfo.stencil_name), "%s", stencil_name.c_str());

				modelStencilList.push_back(modelConfig);
			}
		}
	}

}