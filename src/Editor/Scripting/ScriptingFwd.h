#pragma once

#include <memory>

struct lua_State;
typedef struct lua_State lua_State;

class CommonScriptContext;
using CommonScriptContextPtr = std::shared_ptr<CommonScriptContext>;
using CommonScriptContextWeakPtr = std::weak_ptr<CommonScriptContext>;

class ComponentScriptContext;
using ComponentScriptContextPtr = std::shared_ptr<ComponentScriptContext>;
using ComponentScriptContextWeakPtr = std::weak_ptr<ComponentScriptContext>;
