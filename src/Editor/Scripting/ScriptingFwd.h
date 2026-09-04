#pragma once

#include <memory>

struct lua_State;
typedef struct lua_State lua_State;

class CommonScriptContext;
using CommonScriptContextPtr= std::shared_ptr<CommonScriptContext>;
using CommonScriptContextWeakPtr= std::weak_ptr<CommonScriptContext>;

class ProjectScriptContext;
using ProjectScriptContextPtr= std::shared_ptr<ProjectScriptContext>;
using ProjectScriptContextConstPtr= std::shared_ptr<const ProjectScriptContext>;
using ProjectScriptContextWeakPtr= std::weak_ptr<ProjectScriptContext>;
