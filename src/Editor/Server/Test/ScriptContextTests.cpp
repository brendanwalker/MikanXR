#include "ScriptContextTests.h"
#include "unit_test.h"

#include "CommonScriptContext.h"
#include "PathUtils.h"

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
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_chunk_name_is_project_relative);
	UNIT_TEST_MODULE_CALL_TEST(script_module_test_missing_module_names_scripts_folder);
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
