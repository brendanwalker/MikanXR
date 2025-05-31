#pragma once

struct VRDevicePosition
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct VRDeviceQuat
{
	float w = 1.f;
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

struct VRDevicePose
{
	VRDevicePosition position;
	VRDeviceQuat orientation;
};