#include "ScriptContextTests.h"
#include "unit_test.h"

#include "CommonScriptContext.h"
#include "Light/DMXFixtureComponent.h"
#include "Light/DMXFixtureGroupComponent.h"
#include "Light/RGBPixelGridComponent.h"
#include "MikanComponent.h"
#include "PathUtils.h"
#include "Scene/TransformComponent.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <assert.h>
#include <fstream>
#include <filesystem>
#include <memory>
#include <stdio.h>
#include <string>

namespace
{
// A throwaway project folder holding a scripts/ tree, since the module search
// path is built from whatever project is loaded. The previous project directory
// is restored on the way out so the rest of the suite is unaffected.
class ScopedTestProject
{
public:
	ScopedTestProject()
	{
		m_previousProjectDir= PathUtils::getProjectDirectory();

		m_projectDir= std::filesystem::temp_directory_path() / "mikan_script_module_tests";
		std::filesystem::remove_all(m_projectDir);
		std::filesystem::create_directories(getScriptsDir());

		PathUtils::setProjectDirectory(m_projectDir);
	}

	~ScopedTestProject()
	{
		PathUtils::setProjectDirectory(m_previousProjectDir);

		std::error_code ignored;
		std::filesystem::remove_all(m_projectDir, ignored);
	}

	std::filesystem::path getScriptsDir() const { return m_projectDir / "scripts"; }

	// Write one file under scripts/, creating any folders its relative path names
	std::filesystem::path writeScript(const std::string& relativePath, const std::string& content) const
	{
		const std::filesystem::path filePath= getScriptsDir() / relativePath;
		std::filesystem::create_directories(filePath.parent_path());

		std::ofstream file(filePath, std::ios::binary);
		file << content;

		return filePath;
	}

private:
	std::filesystem::path m_projectDir;
	std::filesystem::path m_previousProjectDir;
};
} // namespace

bool run_script_context_tests()
{
	UNIT_TEST_MODULE_BEGIN("script_context")
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_resolves_project_scripts);
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_resolves_bundled_scripts);
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_chunk_name_is_project_relative);
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_missing_module_names_scripts_folder);
	UNIT_TEST_MODULE_CALL_TEST(script_context_test_pushes_concrete_component_class);
	UNIT_TEST_MODULE_END()
}

bool script_module_test_resolves_project_scripts()
{
	UNIT_TEST_BEGIN("require finds modules in the project scripts folder")

	ScopedTestProject project;
	project.writeScript("test_color.lua", "return { shade = function() return 42 end }\n");
	project.writeScript("test_pkg/sub.lua", "return { name = 'sub' }\n");
	project.writeScript("test_folder/init.lua", "return { name = 'init' }\n");
	const std::filesystem::path callerPath=
		project.writeScript("test_caller.lua", "local color = require('test_color')\n"
											   "local sub = require('test_pkg.sub')\n"
											   "local folder = require('test_folder')\n"
											   "test_shade = color.shade()\n"
											   "test_sub_name = sub.name\n"
											   "test_folder_name = folder.name\n");

	auto context= std::make_shared<CommonScriptContext>();
	success&= context->createScriptState();
	assert(success);
	success&= context->runScriptFile(callerPath, 1);
	assert(success);

	std::string result;
	success&= context->evalString("return test_shade", result) && result == "42";
	assert(success);

	// A dotted name maps to a subfolder, and a folder name to its init.lua
	success&= context->evalString("return test_sub_name", result) && result == "sub";
	assert(success);
	success&= context->evalString("return test_folder_name", result) && result == "init";
	assert(success);

	// The module ran once and its result is cached under the name it was required by
	success&= context->evalString("return package.loaded['test_color'] ~= nil", result) && result == "true";
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_module_test_resolves_bundled_scripts()
{
	UNIT_TEST_BEGIN("require finds bundled modules behind the project scripts folder")

	// The project has no easing.lua of its own; the bundled resources/scripts copy answers
	ScopedTestProject project;
	const std::filesystem::path callerPath=
		project.writeScript("test_caller.lua", "local easing = require('easing')\n"
											   "test_has_easing = type(easing) == 'table'\n");

	auto context= std::make_shared<CommonScriptContext>();
	success&= context->createScriptState();
	assert(success);
	success&= context->runScriptFile(callerPath, 1);
	assert(success);

	std::string result;
	success&= context->evalString("return test_has_easing", result) && result == "true";
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_module_test_chunk_name_is_project_relative()
{
	UNIT_TEST_BEGIN("a required module loads under its project relative path")

	ScopedTestProject project;
	project.writeScript("test_source.lua", "return { source = debug.getinfo(1, 'S').source }\n");
	const std::filesystem::path callerPath=
		project.writeScript("test_caller.lua", "test_source = require('test_source').source\n");

	auto context= std::make_shared<CommonScriptContext>();
	success&= context->createScriptState();
	assert(success);
	success&= context->runScriptFile(callerPath, 1);
	assert(success);

	// The same form runScriptFile assigns, which is what the debugger's
	// source root makes breakpoint paths relative to
	std::string result;
	success&= context->evalString("return test_source", result) && result == "@scripts/test_source.lua";
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_module_test_missing_module_names_scripts_folder()
{
	UNIT_TEST_BEGIN("a missing module reports the project scripts folder")

	ScopedTestProject project;
	// pcall so the failure stays inside the chunk: an uncaught error would
	// dispose the state before the message could be read back
	const std::filesystem::path callerPath=
		project.writeScript("test_caller.lua", "local ok, err = pcall(require, 'test_absent')\n"
											   "test_ok = ok\n"
											   "test_err = err\n");

	auto context= std::make_shared<CommonScriptContext>();
	success&= context->createScriptState();
	assert(success);
	success&= context->runScriptFile(callerPath, 1);
	assert(success);

	std::string result;
	success&= context->evalString("return test_ok", result) && result == "false";
	assert(success);

	const std::string scriptsDir= project.getScriptsDir().generic_string();
	success&= context->evalString("return test_err", result) && result.find(scriptsDir) != std::string::npos;
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool script_context_test_pushes_concrete_component_class()
{
	UNIT_TEST_BEGIN("a component pushes as its concrete class, not the static one")

	auto context= std::make_shared<CommonScriptContext>();
	success&= context->createScriptState();
	assert(success);

	lua_State* L= context->getLuaState();

	// Parent classes before derived ones, as ProjectScriptContext binds them
	MikanComponent::bindLuaFunctions(L);
	TransformComponent::bindLuaFunctions(L);
	DMXFixtureComponent::bindLuaFunctions(L);
	RGBPixelGridComponent::bindLuaFunctions(L);

	context->registerComponentClass(DMXFixtureComponent::k_componentClassName,
									[](lua_State* state, MikanComponentPtr component)
									{
										return static_cast<bool>(luabridge::push(
											state, std::dynamic_pointer_cast<DMXFixtureComponent>(component).get()));
									});
	context->registerComponentClass(RGBPixelGridComponent::k_componentClassName,
									[](lua_State* state, MikanComponentPtr component)
									{
										return static_cast<bool>(luabridge::push(
											state, std::dynamic_pointer_cast<RGBPixelGridComponent>(component).get()));
									});

	auto grid= std::make_shared<RGBPixelGridComponent>(MikanObjectWeakPtr());
	auto gridDefinition= std::make_shared<RGBPixelGridDefinition>();
	gridDefinition->resizeGrid(6, 4);
	grid->setDefinition(gridDefinition);

	// Held as the base type, which is how a group hands its fixtures out
	MikanComponentPtr fixture= grid;
	success&= context->pushComponent(L, fixture);
	assert(success);
	lua_setglobal(L, "test_fixture");

	// columns and rows live on RGBPixelGridComponent. They read back only if
	// the userdata carries that class's metatable rather than the base's.
	std::string result;
	success&= context->evalString("return test_fixture.columns", result) && result == "6";
	assert(success);
	success&= context->evalString("return test_fixture.rows", result) && result == "4";
	assert(success);

	// The inherited surface still resolves through the class hierarchy
	success&= context->evalString("return test_fixture.className", result)
			  && result == RGBPixelGridComponent::k_componentClassName;
	assert(success);

	// A null component is nil rather than a missing global
	success&= context->pushComponent(L, MikanComponentPtr());
	assert(success);
	lua_setglobal(L, "test_absent_fixture");
	success&= context->evalString("return test_absent_fixture == nil", result) && result == "true";
	assert(success);

	// getFixtureAtIndex reaches the context back through the state it is called
	// with. This group has no owner, so nothing resolves and every index is nil,
	// which is enough to exercise that path.
	DMXFixtureGroupComponent::bindLuaFunctions(L);
	auto group= std::make_shared<DMXFixtureGroupComponent>(MikanObjectWeakPtr());
	group->setDefinition(std::make_shared<DMXFixtureGroupDefinition>());
	success&= static_cast<bool>(luabridge::setGlobal(L, group.get(), "test_group"));
	assert(success);
	success&= context->evalString("return test_group:getFixtureAtIndex(0) == nil", result) && result == "true";
	assert(success);

	UNIT_TEST_COMPLETE()
}
