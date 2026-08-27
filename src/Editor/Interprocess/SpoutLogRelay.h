#pragma once

#include <filesystem>
#include <string>

// Routes Spout's own diagnostics into the Mikan log.
//
// Spout 2.007 exposes no log callback, and its console logging allocates a second
// window on top of the editor. The relay points Spout's file logging at a file of our
// own instead and tails it every tick, so the lines reach the log panel and MikanXR.log
// like any other editor output. Disabled by default; the MIKAN_SPOUT_LOG environment
// variable used by the shared texture writers overrides it when set.
class SpoutLogRelay
{
public:
	SpoutLogRelay();
	~SpoutLogRelay();

	void setEnabled(bool bEnabled);
	inline bool getEnabled() const { return m_bEnabled; }

	// Emits whatever Spout has appended since the last call
	void update();

private:
	void drain();

	std::filesystem::path m_logPath;
	// Spout appends one line at a time, but a read can still land mid-line
	std::string m_partialLine;
	std::streamoff m_readOffset= 0;
	struct SPOUTLIBRARY* m_spoutLibrary= nullptr;
	bool m_bEnabled= false;
};
