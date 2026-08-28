#pragma once

// In-app view of the editor log, replacing the Win32 console window. Reads the
// ring buffer the automation server's "log tail" command already fills, rather
// than installing a second log sink.
class LogPanel
{
public:
	static LogPanel& getInstance();

	void draw(bool* pOpen);

private:
	bool m_bAutoScroll= true;
	// A LogSeverityLevel value; the editor is chatty at debug and below
	int m_minLevel= 0;
};
