#pragma once

#include "../MikanAPITypes.h"

#include <type_traits>
#include <Refureku/TypeInfo/Entity/DefaultEntityRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/ArchetypeRegisterer.h>
#include <Refureku/TypeInfo/Namespace/Namespace.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragment.h>
#include <Refureku/TypeInfo/Namespace/NamespaceFragmentRegisterer.h>
#include <Refureku/TypeInfo/Archetypes/Template/TypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/NonTypeTemplateParameter.h>
#include <Refureku/TypeInfo/Archetypes/Template/TemplateTemplateParameter.h>

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_1095719431187359814u = MikanRequest::staticGetArchetype(); }

rfk::Struct const& MikanRequest::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MikanRequest", 1095719431187359814u, sizeof(MikanRequest), 0);
if (!initialized) {
initialized = true;
MikanRequest::_rfk_registerChildClass<MikanRequest>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MikanRequest>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MikanRequest>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MikanRequest>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MikanRequest>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MikanRequest>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MikanRequest>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MikanRequest>() noexcept { return &MikanRequest::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_7094118849615581562u = MikanResponse::staticGetArchetype(); }

rfk::Struct const& MikanResponse::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MikanResponse", 7094118849615581562u, sizeof(MikanResponse), 0);
if (!initialized) {
initialized = true;
MikanResponse::_rfk_registerChildClass<MikanResponse>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MikanResponse>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MikanResponse>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MikanResponse>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MikanResponse>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MikanResponse>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MikanResponse>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MikanResponse>() noexcept { return &MikanResponse::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_8521159033538382795u = MikanEvent::staticGetArchetype(); }

rfk::Struct const& MikanEvent::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MikanEvent", 8521159033538382795u, sizeof(MikanEvent), 0);
if (!initialized) {
initialized = true;
MikanEvent::_rfk_registerChildClass<MikanEvent>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MikanEvent>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MikanEvent>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MikanEvent>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MikanEvent>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MikanEvent>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MikanEvent>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MikanEvent>() noexcept { return &MikanEvent::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_16144647093330338146u = MikanClientInfo::staticGetArchetype(); }

rfk::Struct const& MikanClientInfo::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MikanClientInfo", 16144647093330338146u, sizeof(MikanClientInfo), 0);
if (!initialized) {
initialized = true;
MikanClientInfo::_rfk_registerChildClass<MikanClientInfo>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MikanClientInfo>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MikanClientInfo>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MikanClientInfo>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MikanClientInfo>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MikanClientInfo>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MikanClientInfo>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MikanClientInfo>() noexcept { return &MikanClientInfo::staticGetArchetype(); }

namespace rfk::generated { static rfk::ArchetypeRegisterer const registerer_11994515914276737111u = MikanColorRGB::staticGetArchetype(); }

rfk::Struct const& MikanColorRGB::staticGetArchetype() noexcept {
static bool initialized = false;
static rfk::Struct type("MikanColorRGB", 11994515914276737111u, sizeof(MikanColorRGB), 0);
if (!initialized) {
initialized = true;
MikanColorRGB::_rfk_registerChildClass<MikanColorRGB>(type);
static rfk::StaticMethod defaultSharedInstantiator("", 0u, rfk::getType<rfk::SharedPtr<MikanColorRGB>>(),new rfk::NonMemberFunction<rfk::SharedPtr<MikanColorRGB>()>(&rfk::internal::CodeGenerationHelpers::defaultSharedInstantiator<MikanColorRGB>),rfk::EMethodFlags::Default, nullptr);
type.addSharedInstantiator(defaultSharedInstantiator);
static rfk::StaticMethod defaultUniqueInstantiator("", 0u, rfk::getType<rfk::UniquePtr<MikanColorRGB>>(),new rfk::NonMemberFunction<rfk::UniquePtr<MikanColorRGB>()>(&rfk::internal::CodeGenerationHelpers::defaultUniqueInstantiator<MikanColorRGB>),rfk::EMethodFlags::Default, nullptr);
type.addUniqueInstantiator(defaultUniqueInstantiator);
type.setMethodsCapacity(0u); type.setStaticMethodsCapacity(0u); 
}
return type; }

template <> rfk::Archetype const* rfk::getArchetype<MikanColorRGB>() noexcept { return &MikanColorRGB::staticGetArchetype(); }


