/**
\file
*/

#pragma once

#include "MikanCoreTypes.h"
#include "MikanMathTypes.h"

#include <stdint.h>
#include <string>
#include <vector>

//cut_before

/**
\brief Client Interface for MikanXR
\defgroup MikanClientTypes Serializable types used for client API
\addtogroup MikanClientTypes
@{
*/

// Wrapper Types
//--------------

/// The ID of a VR Device
using MikanVRDeviceID= int32_t;

/// The ID of a stencil
using MikanStencilID= int32_t;

/// The ID of a spatial anchor
using MikanSpatialAnchorID= int32_t;

#define INVALID_MIKAN_ID				-1
#define ORIGIN_SPATIAL_ANCHOR_NAME		"Origin"


// Shared Constants
//-----------------

enum MikanVideoSourceType
{
	MikanVideoSourceType_MONO,
	MikanVideoSourceType_STEREO
};

/// The list of possible camera APIs used by MikanXR
enum MikanVideoSourceApi
{
	MikanVideoSourceApi_INVALID,
	MikanVideoSourceApi_OPENCV_CV,
	MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION
};

/// The list of possible vr device drivers used by MikanXR Client API
enum MikanVRDeviceApi
{
	MikanVRDeviceApi_INVALID,
	MikanVRDeviceApi_STEAM_VR,
};

/// The list of possible vr device types used by MikanXR Client API
enum MikanVRDeviceType
{
	MikanVRDeviceType_INVALID,
	MikanVRDeviceType_HMD,
	MikanVRDeviceType_CONTROLLER,
	MikanVRDeviceType_TRACKER
};

struct MikanRequest
{
	inline static const std::string k_typeName = "MikanRequest";

	std::string requestType;
	MikanRequestID requestId;
	int version;

	MikanRequest() = default;
	MikanRequest(const std::string& inRequestType) : requestType(inRequestType) {}
};

struct MikanResponse
{
	inline static const std::string k_typeName = "MikanResponse";

	std::string responseType;
	MikanRequestID requestId;
	MikanResult resultCode;

	MikanResponse() = default;
	MikanResponse(const std::string& inResponseType) : responseType(inResponseType) {}
};

struct MikanEvent
{
	std::string eventType;

	MikanEvent()= default;
	MikanEvent(const std::string& inEventType) : eventType(inEventType) {}
};

struct MikanClientInfo
{
	inline static const std::string k_typeName = "MikanClientInfo";

	bool supportsRBG24;
	bool supportsRBGA32;
	bool supportsBGRA32;
	bool supportsDepth;
	std::string engineName;
	std::string engineVersion;
	std::string applicationName;
	std::string applicationVersion;
	std::string xrDeviceName;
	int mikanCoreSdkVersion;
	MikanClientGraphicsApi graphicsAPI;
};

/// A float RGB color with [0,1] components.
struct MikanColorRGB
{
	inline static const std::string k_typeName = "MikanColorRGB";

	float r, g, b;
};

struct MikanStencilQuad : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilQuad";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float quad_width;
	float quad_height;
	bool is_double_sided;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilQuad() : MikanResponse(k_typeName) {}
};

struct MikanStencilBox : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilBox";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	float box_x_size;
	float box_y_size;
	float box_z_size;
	bool is_disabled;
	std::string stencil_name;

	MikanStencilBox() : MikanResponse(k_typeName) {}
};

struct MikanStencilModel : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilModel";

	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanTransform relative_transform; // transform relative to parent anchor
	bool is_disabled;
	std::string stencil_name;

	MikanStencilModel() : MikanResponse(k_typeName) {}
};

struct MikanStencilList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanStencilList";

	std::vector<MikanStencilID> stencil_id_list;

	MikanStencilList() : MikanResponse(k_typeName) {}
};

// VR Device Response Types
struct MikanVRDeviceList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceList";

	std::vector<MikanStencilID> vr_device_id_list;

	MikanVRDeviceList() : MikanResponse(k_typeName) {}
};

struct MikanVRDeviceInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceInfo";

	MikanVRDeviceApi vr_device_api;
	MikanVRDeviceType vr_device_type;
	std::string device_path;

	MikanVRDeviceInfo() : MikanResponse(k_typeName) {}
};

// Spatial Anchor Response Types
struct MikanSpatialAnchorList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorList";

	std::vector<MikanSpatialAnchorID> spatial_anchor_id_list;

	MikanSpatialAnchorList() : MikanResponse(k_typeName) {}
};

struct MikanSpatialAnchorInfo  : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorInfo";

	MikanSpatialAnchorID anchor_id;
	MikanTransform world_transform; // Transform in tracking system space
	std::string anchor_name;

	MikanSpatialAnchorInfo() : MikanResponse(k_typeName) {}
};

// Script Event Types
struct MikanScriptMessageInfo : public MikanEvent
{
	inline static const std::string k_typeName = "MikanScriptMessageInfo";

	std::string content;

	MikanScriptMessageInfo() : MikanEvent(k_typeName) {}
};

/**
@}
*/

//cut_after
