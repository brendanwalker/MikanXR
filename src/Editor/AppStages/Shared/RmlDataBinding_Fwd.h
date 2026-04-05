#pragma once

#include <memory>
#include <string>

template <typename t_element_type> class RmlDataBinding_List;

using RmlDataBinding_IntList = RmlDataBinding_List<int>;
using RmlDataBinding_IntListPtr = std::shared_ptr<RmlDataBinding_IntList>;

using RmlDataBinding_ComponentIdList = RmlDataBinding_List<int>;
using RmlDataBinding_ComponentIdListPtr = std::shared_ptr<RmlDataBinding_ComponentIdList>;

using RmlDataBinding_ScriptTriggerList = RmlDataBinding_List<std::string>;
using RmlDataBinding_ScriptTriggerListPtr = std::shared_ptr<RmlDataBinding_ScriptTriggerList>;

class RmlModel_MikanComponent;
using RmlModel_MikanComponentPtr = std::shared_ptr<RmlModel_MikanComponent>;

class RmlModel_EntityAccessor;
using RmlModel_EntityAccessorPtr = std::shared_ptr<RmlModel_EntityAccessor>;
