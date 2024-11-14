#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanRenderTargetRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRenderTargetRequest")) AllocateRenderTargetTextures : 
	public MikanRequest
{
public:
	inline static const char* k_typeName = "AllocateRenderTargetTextures";
	AllocateRenderTargetTextures() : MikanRequest(k_typeName) {}

	FIELD()
	MikanRenderTargetDescriptor descriptor;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	AllocateRenderTargetTextures_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRenderTargetRequest")) WriteColorRenderTargetTexture :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "WriteColorRenderTargetTexture";
	WriteColorRenderTargetTexture() : MikanRequest(k_typeName) {}

	FIELD()
	void* apiColorTexturePtr;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	WriteColorRenderTargetTexture_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRenderTargetRequest")) WriteDepthRenderTargetTexture :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "WriteDepthRenderTargetTexture";
	WriteDepthRenderTargetTexture() : MikanRequest(k_typeName) {}

	FIELD()
	void* apiDepthTexturePtr;

	FIELD()
	float zNear;

	FIELD()
	float zFar;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	WriteDepthRenderTargetTexture_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRenderTargetRequest")) PublishRenderTargetTextures :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "WriteDepthRenderTargetTexture";
	PublishRenderTargetTextures() : MikanRequest(k_typeName) {}

	FIELD()
	uint64_t frameIndex;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	PublishRenderTargetTextures_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanRenderTargetRequest")) FreeRenderTargetTextures :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "FreeRenderTargetTextures";
	FreeRenderTargetTextures() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	FreeRenderTargetTextures_GENERATED
	#endif
};


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanRenderTargetRequests_GENERATED
#endif