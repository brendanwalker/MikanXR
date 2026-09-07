#include "SharedTextureWriterBackend.h"
#include "SpoutUtils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

// The Spout log verbs are free functions in this namespace, which SpoutDX.h opens for its own users
using namespace spoututils;

// The log policy the backends share. It lives in one translation unit so its apply-once guard
// is one guard for the whole process rather than one per API file.

const char* const k_spoutSenderLogFileName= "MikanSpoutSender.log";

SpoutLogTarget getSpoutLogTarget()
{
	const char* setting= std::getenv("MIKAN_SPOUT_LOG");
	if (setting == nullptr)
		return SpoutLogTarget::none;

	std::string value(setting);
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return (char)std::tolower(c); });

	if (value == "console" || value == "1" || value == "on" || value == "true")
		return SpoutLogTarget::console;
	if (value == "file")
		return SpoutLogTarget::file;

	return SpoutLogTarget::none;
}

void applySpoutDXLogPolicy()
{
	static bool s_bPolicyApplied= false;
	if (s_bPolicyApplied)
		return;
	s_bPolicyApplied= true;

	switch (getSpoutLogTarget())
	{
	case SpoutLogTarget::console:
		EnableSpoutLog();
		break;
	case SpoutLogTarget::file:
		EnableSpoutLogFile(k_spoutSenderLogFileName);
		break;
	case SpoutLogTarget::none:
		return;
	}

	SetSpoutLogLevel(SpoutLogLevel::SPOUT_LOG_VERBOSE);
}
