#pragma once

#include "MikanMathTypes.h"

void MikanOrientationToEulerAngles(
	const MikanVector3f& x_axis, const MikanVector3f& y_axis, const MikanVector3f& z_axis,
	float& out_x_angle, float& out_y_angle, float& out_z_angle);
void EulerAnglesToMikanOrientation(
	const float x_angle, const float y_angle, const float z_angle,
	MikanVector3f& out_x_axis, MikanVector3f& out_y_axis, MikanVector3f& out_z_axis);
