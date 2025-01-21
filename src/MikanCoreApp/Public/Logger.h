#pragma once

//-- includes -----
#include <string>
#include <sstream>

#include "MikanCoreAppExport.h"

//-- constants -----
enum class LogSeverityLevel : int
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

//-- classes -----
class MIKAN_COREAPP_CLASS LoggerStream
{
protected:
	std::ostringstream m_lineBuffer;
	LogSeverityLevel m_level;
	bool m_hasWrittenLog= false;

public:
	LoggerStream(LogSeverityLevel level);
	virtual ~LoggerStream();

	// accepts just about anything
	template<class T>
	LoggerStream &operator<<(const T &x)
	{
		if (log_can_emit_level(m_level))
		{
			m_lineBuffer << x;
			m_hasWrittenLog= true;
		}

		return *this;
	}

protected:
	virtual void write_line();
};

class MIKAN_COREAPP_CLASS ThreadSafeLoggerStream : public LoggerStream
{
public:
	ThreadSafeLoggerStream(LogSeverityLevel level);

protected:
	void write_line() override;
};

typedef void (*t_logCallback)(int, const char*);

struct LoggerSettings
{
	LogSeverityLevel min_log_level;
	std::string log_filename;
	bool enable_console;
	t_logCallback log_callback;
};

//-- interface -----
MIKAN_COREAPP_FUNC(void) log_init(const LoggerSettings& settings);
MIKAN_COREAPP_FUNC(void) log_dispose();
MIKAN_COREAPP_FUNC(bool) log_can_emit_level(LogSeverityLevel level);
MIKAN_COREAPP_FUNC(std::string) log_get_timestamp_prefix();

//-- macros -----
#define SELECT_LOG_STREAM(level) LoggerStream(level)
#define SELECT_MT_LOG_STREAM(level) ThreadSafeLoggerStream(level)

// Non Thread Safe Logger Macros
// Almost everything is on the main thread, so you almost always want to use these
#define MIKAN_LOG_TRACE(function_name) SELECT_LOG_STREAM(LogSeverityLevel::trace) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_DEBUG(function_name) SELECT_LOG_STREAM(LogSeverityLevel::debug) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_INFO(function_name) SELECT_LOG_STREAM(LogSeverityLevel::info) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_WARNING(function_name) SELECT_LOG_STREAM(LogSeverityLevel::warning) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_ERROR(function_name) SELECT_LOG_STREAM(LogSeverityLevel::error) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_FATAL(function_name) SELECT_LOG_STREAM(LogSeverityLevel::fatal) << log_get_timestamp_prefix() << function_name << " - "

// Thread Safe Logger Macros
// Uses thread safe locking before appending data to the logging stream
// Only use this when logging from other threads
#define MIKAN_MT_LOG_TRACE(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::trace) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_DEBUG(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::debug) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_INFO(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::info) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_WARNING(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::warning) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_ERROR(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::error) << log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_FATAL(function_name) SELECT_MT_LOG_STREAM(LogSeverityLevel::fatal) << log_get_timestamp_prefix() << function_name << " - "

