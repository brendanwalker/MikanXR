#pragma once

#include "MikanMathTypes.h"
#include "nlohmann/json.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVector3d, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVector2f, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanVector3f, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanRotator3f, x_angle, y_angle, z_angle)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanMatrix4f, x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3, w0, w1, w2, w3)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanMatrix3d, x0, x1, x2, y0, y1, y2, z0, z1, z2)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanMatrix4x3d, x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanMatrix4d, x0, x1, x2, x3, y0, y1, y2, y3, z0, z1, z2, z3, w0, w1, w2, w3)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanQuatf, w, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanQuatd, w, x, y, z)

// MikanTransform
void to_json(nlohmann::json& j, const MikanTransform& p)
{
	j = nlohmann::json{{"scale", p.scale}, {"rotation", p.rotation}, {"position", p.position}};
}
void from_json(const nlohmann::json& j, MikanTransform& p)
{
	j.at("scale").get_to(p.scale);
	j.at("rotation").get_to(p.rotation);
	j.at("position").get_to(p.position);
}
