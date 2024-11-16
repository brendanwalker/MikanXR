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
	inline static const char* k_typeName = "MikanStencilNameChangeEvent";

	FIELD()
	MikanStencilID stencil_id;
	FIELD()
	Serialization::String stencil_name;

	MikanStencilNameUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilNameUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanStencilPoseUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanStencilPoseUpdateEvent";

	FIELD()
	MikanTransform transform;
	FIELD()
	MikanStencilID stencil_id;

	MikanStencilPoseUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStencilPoseUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanQuadStencilListUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanQuadStencilListUpdateEvent";

	MikanQuadStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanQuadStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanBoxStencilListUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanBoxStencilListUpdateEvent";

	MikanBoxStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanBoxStencilListUpdateEvent_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanStencilEvents")) MikanModelStencilListUpdateEvent : public MikanEvent
{
	inline static const char* k_typeName = "MikanModelStencilListUpdateEvent";

	MikanModelStencilListUpdateEvent() : MikanEvent(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanModelStencilListUpdateEvent_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanStencilEvents_GENERATED
#endif