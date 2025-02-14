#pragma once

#include <memory>

//-- predeclarations -----
struct lua_State;
typedef struct lua_State lua_State;

class CommonScriptContext;
using CommonScriptContextPtr = std::shared_ptr<CommonScriptContext>;
using CommonScriptContextWeakPtr = std::weak_ptr<CommonScriptContext>;

class CompositorScriptContext;
using CompositorScriptContextPtr = std::shared_ptr<CompositorScriptContext>;
using CompositorScriptContextWeakPtr = std::weak_ptr<CompositorScriptContext>;
