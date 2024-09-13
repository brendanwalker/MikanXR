#pragma once

#include "EntityMacros.h"

#include <Refureku/TypeInfo/Entity/EEntityKind.h>
#include <string>
#include <Refureku/Misc/CodeGenerationHelpers.h>
#include <Refureku/Misc/DisableWarningMacros.h>
#include <Refureku/TypeInfo/Functions/Method.h>
#include <Refureku/TypeInfo/Functions/StaticMethod.h>
#include <Refureku/TypeInfo/Variables/Field.h>
#include <Refureku/TypeInfo/Variables/StaticField.h>
#include <Refureku/TypeInfo/Archetypes/Enum.h>
#include <Refureku/TypeInfo/Archetypes/EnumValue.h>
#include <Refureku/TypeInfo/Variables/Variable.h>
#include <Refureku/TypeInfo/Functions/Function.h>
#include <Refureku/TypeInfo/Archetypes/Template/ClassTemplate.h>
#include <Refureku/TypeInfo/Archetypes/Template/ClassTemplateInstantiation.h>
#include <Refureku/TypeInfo/Archetypes/Template/ClassTemplateInstantiationRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateArgument.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateArgument.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateArgument.h>


#define MikanRequest_GENERATED	\
RFK_UNPACK_IF_NOT_PARSING(friend rfk::internal::CodeGenerationHelpers;\
friend rfk::internal::implements_template1__rfk_registerChildClass<MikanRequest, void, void(rfk::Struct&)>; \
\
private: template <typename ChildClass> static void _rfk_registerChildClass(rfk::Struct& childClass) noexcept {\
rfk::Struct const& thisClass = staticGetArchetype();\
if constexpr (!std::is_same_v<ChildClass, MikanRequest>)const_cast<rfk::Struct&>(thisClass).addSubclass(childClass, rfk::internal::CodeGenerationHelpers::computeClassPointerOffset<ChildClass, MikanRequest>());\
else\
{\
childClass.setFieldsCapacity(0u + 0); childClass.setStaticFieldsCapacity(0u + 0); \
}\
}\
\
public:  static rfk::Struct const& staticGetArchetype() noexcept;\
\
)\


#define MikanResponse_GENERATED	\
RFK_UNPACK_IF_NOT_PARSING(friend rfk::internal::CodeGenerationHelpers;\
friend rfk::internal::implements_template1__rfk_registerChildClass<MikanResponse, void, void(rfk::Struct&)>; \
\
private: template <typename ChildClass> static void _rfk_registerChildClass(rfk::Struct& childClass) noexcept {\
rfk::Struct const& thisClass = staticGetArchetype();\
if constexpr (!std::is_same_v<ChildClass, MikanResponse>)const_cast<rfk::Struct&>(thisClass).addSubclass(childClass, rfk::internal::CodeGenerationHelpers::computeClassPointerOffset<ChildClass, MikanResponse>());\
else\
{\
childClass.setFieldsCapacity(0u + 0); childClass.setStaticFieldsCapacity(0u + 0); \
}\
}\
\
public:  static rfk::Struct const& staticGetArchetype() noexcept;\
\
)\


#define MikanEvent_GENERATED	\
RFK_UNPACK_IF_NOT_PARSING(friend rfk::internal::CodeGenerationHelpers;\
friend rfk::internal::implements_template1__rfk_registerChildClass<MikanEvent, void, void(rfk::Struct&)>; \
\
private: template <typename ChildClass> static void _rfk_registerChildClass(rfk::Struct& childClass) noexcept {\
rfk::Struct const& thisClass = staticGetArchetype();\
if constexpr (!std::is_same_v<ChildClass, MikanEvent>)const_cast<rfk::Struct&>(thisClass).addSubclass(childClass, rfk::internal::CodeGenerationHelpers::computeClassPointerOffset<ChildClass, MikanEvent>());\
else\
{\
childClass.setFieldsCapacity(0u + 0); childClass.setStaticFieldsCapacity(0u + 0); \
}\
}\
\
public:  static rfk::Struct const& staticGetArchetype() noexcept;\
\
)\


#define MikanClientInfo_GENERATED	\
RFK_UNPACK_IF_NOT_PARSING(friend rfk::internal::CodeGenerationHelpers;\
friend rfk::internal::implements_template1__rfk_registerChildClass<MikanClientInfo, void, void(rfk::Struct&)>; \
\
private: template <typename ChildClass> static void _rfk_registerChildClass(rfk::Struct& childClass) noexcept {\
rfk::Struct const& thisClass = staticGetArchetype();\
if constexpr (!std::is_same_v<ChildClass, MikanClientInfo>)const_cast<rfk::Struct&>(thisClass).addSubclass(childClass, rfk::internal::CodeGenerationHelpers::computeClassPointerOffset<ChildClass, MikanClientInfo>());\
else\
{\
childClass.setFieldsCapacity(0u + 0); childClass.setStaticFieldsCapacity(0u + 0); \
}\
}\
\
public:  static rfk::Struct const& staticGetArchetype() noexcept;\
\
)\


#define MikanColorRGB_GENERATED	\
RFK_UNPACK_IF_NOT_PARSING(friend rfk::internal::CodeGenerationHelpers;\
friend rfk::internal::implements_template1__rfk_registerChildClass<MikanColorRGB, void, void(rfk::Struct&)>; \
\
private: template <typename ChildClass> static void _rfk_registerChildClass(rfk::Struct& childClass) noexcept {\
rfk::Struct const& thisClass = staticGetArchetype();\
if constexpr (!std::is_same_v<ChildClass, MikanColorRGB>)const_cast<rfk::Struct&>(thisClass).addSubclass(childClass, rfk::internal::CodeGenerationHelpers::computeClassPointerOffset<ChildClass, MikanColorRGB>());\
else\
{\
childClass.setFieldsCapacity(0u + 0); childClass.setStaticFieldsCapacity(0u + 0); \
}\
}\
\
public:  static rfk::Struct const& staticGetArchetype() noexcept;\
\
)\


#define File_MikanAPITypes_GENERATED	\
template <>  rfk::Archetype const* rfk::getArchetype<MikanRequest>() noexcept;\
template <>  rfk::Archetype const* rfk::getArchetype<MikanResponse>() noexcept;\
template <>  rfk::Archetype const* rfk::getArchetype<MikanEvent>() noexcept;\
template <>  rfk::Archetype const* rfk::getArchetype<MikanClientInfo>() noexcept;\
template <>  rfk::Archetype const* rfk::getArchetype<MikanColorRGB>() noexcept;\


