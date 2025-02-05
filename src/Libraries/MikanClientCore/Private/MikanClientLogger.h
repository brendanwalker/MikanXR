#pragma once

//-- includes -----
#include <string>
#include <filesystem>

//-- constants -----
enum class ClientLogSeverityLevel : int
{
	trace,
	debug,
	info,
	warning,
	error,
	fatal
};

//-- classes -----
class ClientLoggerStream
{
protected:
	class ClientLoggerStreamImpl* m_impl;

public:
	ClientLoggerStream(ClientLogSeverityLevel level);
	virtual ~ClientLoggerStream();

	ClientLoggerStream& operator<<(bool value);
	ClientLoggerStream& operator<<(char value);
	ClientLoggerStream& operator<<(short value);
	ClientLoggerStream& operator<<(unsigned short value);
	ClientLoggerStream& operator<<(int value);
	ClientLoggerStream& operator<<(unsigned int value);
	ClientLoggerStream& operator<<(long value);
	ClientLoggerStream& operator<<(unsigned long value);
	ClientLoggerStream& operator<<(long long value);
	ClientLoggerStream& operator<<(unsigned long long value);
	ClientLoggerStream& operator<<(float value);
	ClientLoggerStream& operator<<(double value);
	ClientLoggerStream& operator<<(long double value);
	ClientLoggerStream& operator<<(const void* value);
	ClientLoggerStream& operator<<(const char* value);
	ClientLoggerStream& operator<<(const std::string& value);
	ClientLoggerStream& operator<<(const std::filesystem::path& value);

protected:
	virtual void write_line();
};

class ThreadSafeClientLoggerStream : public ClientLoggerStream
{
public:
	ThreadSafeClientLoggerStream(ClientLogSeverityLevel level);

protected:
	void write_line() override;
};

typedef void (*t_logCallback)(int, const char*);

struct ClientLoggerSettings
{
	ClientLogSeverityLevel min_log_level;
	t_logCallback log_callback;
};

//-- interface -----
void client_log_init(const ClientLoggerSettings& settings);
void client_log_dispose();
bool client_log_can_emit_level(ClientLogSeverityLevel level);
std::string client_log_get_timestamp_prefix();

//-- macros -----
#define SELECT_LOG_STREAM(level) ClientLoggerStream(level)
#define SELECT_MT_LOG_STREAM(level) ThreadSafeClientLoggerStream(level)

// Non Thread Safe Logger Macros
// Almost everything is on the main thread, so you almost always want to use these
#define MIKAN_LOG_TRACE(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::trace) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_DEBUG(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::debug) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_INFO(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::info) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_WARNING(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::warning) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_ERROR(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::error) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_LOG_FATAL(function_name) SELECT_LOG_STREAM(ClientLogSeverityLevel::fatal) << client_log_get_timestamp_prefix() << function_name << " - "

// Thread Safe Logger Macros
// Uses thread safe locking before appending data to the logging stream
// Only use this when logging from other threads
#define MIKAN_MT_LOG_TRACE(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::trace) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_DEBUG(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::debug) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_INFO(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::info) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_WARNING(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::warning) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_ERROR(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::error) << client_log_get_timestamp_prefix() << function_name << " - "
#define MIKAN_MT_LOG_FATAL(function_name) SELECT_MT_LOG_STREAM(ClientLogSeverityLevel::fatal) << client_log_get_timestamp_prefix() << function_name << " - "

