#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableString.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanStencilEvents.rfkh.h"
#endif

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanStencilNameUpdateEvent : public MikanEvent
{
	FIELD()
	MikanStencilID stencil_id;
	FIELD()
	Serialization::String stencil_name;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilNameUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanStencilPoseUpdateEvent : public MikanEvent
{
	FIELD()
	MikanTransform transform;
	FIELD()
	MikanStencilID stencil_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilPoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanQuadStencilListUpdateEvent : public MikanEvent
{
	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanQuadStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanBoxStencilListUpdateEvent : public MikanEvent
{
	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoxStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanModelStencilListUpdateEvent : public MikanEvent
{
	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanModelStencilListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilEvents_GENERATED
#endif