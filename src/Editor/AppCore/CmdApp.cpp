#include "CmdApp.h"
#include "Logger.h"
#include "TypeRegistry.h"
#include "TrackerPoseCalibratorTests.h"
#include "ClientApiPropertySchemaTests.h"

#include <cstdio>
#include <cstdlib>

namespace
{
	bool run_all_editor_unit_tests()
	{
		bool success = true;
		success &= run_tracker_pose_calibrator_unit_tests();
		success &= run_client_api_property_schema_tests();
		// Future: add more test modules here
		return success;
	}
}

int CmdApp::exec(int argc, char** argv)
{
	// Run with unbuffered stdout so test progress survives a hard crash.
	// When stdout is a pipe (e.g. CI) it is fully buffered by default, so a
	// crash mid-test would discard all buffered fprintf output and hide which
	// test was running. Unbuffered output makes the last printed line the
	// crash site.
	setvbuf(stdout, nullptr, _IONBF, 0);

	parseCommandLine(argc, argv);

	// Command output is written to stdout directly
	LoggerSettings settings = {};
	settings.min_log_level = LogSeverityLevel::debug;
	settings.enable_console = false;
	settings.log_filename = "MikanCmd.log";
	log_init(settings);

	// Build the reflection type registry 
	// (Used by the serialization-based commands such as the unit tests).
	Serialization::TypeRegistry::buildFromRfkDatabase();

	// Dispatch to the requested command. 
	int result = EXIT_SUCCESS;
	if (hasCommandLineFlag("runTests"))
	{
		result = runTests();
	}
	else
	{
		printUsage();
		result = EXIT_FAILURE;
	}

	log_dispose();

	return result;
}

void CmdApp::parseCommandLine(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.size() > 1 && arg[0] == '-')
		{
			std::string key = arg.substr(1); // strip leading '-'
			auto eqPos = key.find('=');
			if (eqPos != std::string::npos)
				m_commandLineParams[key.substr(0, eqPos)] = key.substr(eqPos + 1);
			else
				m_commandLineFlags.insert(key);
		}
	}
}

bool CmdApp::hasCommandLineFlag(const std::string& flag) const
{
	return m_commandLineFlags.count(flag) > 0;
}

std::string CmdApp::getCommandLineStringArg(const std::string& key, const std::string& defaultValue) const
{
	auto it = m_commandLineParams.find(key);
	return it != m_commandLineParams.end() ? it->second : defaultValue;
}

void CmdApp::printUsage() const
{
	fprintf(stdout,
		"MikanCmd - Mikan command-line tool\n"
		"\n"
		"Usage: MikanCmd <command>\n"
		"\n"
		"Commands:\n"
		"  -runTests    Run the editor unit test suites\n");
}

int CmdApp::runTests() const
{
	const bool testsPassed = run_all_editor_unit_tests();

	return testsPassed ? EXIT_SUCCESS : EXIT_FAILURE;
}
