#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanScriptTypes.h"
#include "SerializableMap.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanScriptRequests.rfkh.h"
#endif

// Triggers are project-wide: every script file runs in the project's one Lua
// state, so a trigger is addressed by name alone. The arguments arrive at the
// Lua trigger function as a table, and may be empty.
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) InvokeScriptTrigger : public MikanRequest
{
public:
	InvokeScriptTrigger(){MIKAN_REQUEST_TYPE_INFO_INIT(InvokeScriptTrigger)}

	FIELD() Serialization::String trigger_name;
	FIELD() Serialization::Map<Serialization::String, Serialization::String> trigger_args;

#ifdef MIKANAPI_REFLECTION_ENABLED
	InvokeScriptTrigger_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) SendScriptMessage : public MikanRequest
{
public:
	SendScriptMessage(){MIKAN_REQUEST_TYPE_INFO_INIT(SendScriptMessage)}

	FIELD() MikanScriptMessageInfo message;

#ifdef MIKANAPI_REFLECTION_ENABLED
	SendScriptMessage_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanScriptRequests_GENERATED
#endif