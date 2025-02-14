//-- includes -----
#include "MikanClientLogger.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <ostream>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': localtime
#endif

#ifdef WIN32
#include <windows.h>
#endif

//-- globals -----
bool g_is_initialized= false;
ClientLogSeverityLevel g_min_log_level= ClientLogSeverityLevel::info;
std::mutex* g_logger_mutex = nullptr;
t_logCallback g_logger_callback = nullptr;

static void log_default_callback(int log_level, const char* line)
{
	// Drop the log on the floor
}

//-- public implementation -----
void client_log_init(const ClientLoggerSettings& settings)
{
	if (!g_is_initialized)
	{
		g_min_log_level = settings.min_log_level;

		if (settings.log_callback != nullptr)
		{
			g_logger_callback= settings.log_callback;
		}
		else
		{
			g_logger_callback= log_default_callback;
		}
		
		g_logger_mutex = new std::mutex();

		g_is_initialized= true;
	}
}

void client_log_dispose()
{
	if (g_logger_mutex != nullptr)
	{
		delete g_logger_mutex;
		g_logger_mutex = nullptr;
	}

	g_is_initialized = false;
}

bool client_log_can_emit_level(ClientLogSeverityLevel level)
{
    return (level >= g_min_log_level);
}

std::string client_log_get_timestamp_prefix()
{
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds);
    time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S") << "." << milliseconds.count() << "]: ";

    return ss.str();
}

//-- member functions -----
class ClientLoggerStreamImpl
{
private:
	std::ostringstream m_lineBuffer;
	ClientLogSeverityLevel m_level;
	bool m_hasWrittenLog;

public:
	ClientLoggerStreamImpl(ClientLogSeverityLevel level) 
		: m_lineBuffer()
		, m_level(level)
		, m_hasWrittenLog(false)
	{
	}

	template<class T>
	void operator<<(const T &x)
	{
		if (client_log_can_emit_level(m_level))
		{
			m_lineBuffer << x;
			m_hasWrittenLog= true;
		}
	}

	void write_line()
	{
		if (g_is_initialized &&
			g_logger_callback != nullptr &&
			m_hasWrittenLog &&
			client_log_can_emit_level(m_level))
		{
			const std::string line = m_lineBuffer.str();

			(*g_logger_callback)((int)m_level, line.c_str());
		}
	}

};

ClientLoggerStream::ClientLoggerStream(ClientLogSeverityLevel level) 
	: m_impl(new ClientLoggerStreamImpl(level))
{
}

ClientLoggerStream::~ClientLoggerStream()
{
	write_line();
	delete m_impl;
}

void ClientLoggerStream::write_line()
{
	m_impl->write_line();
}

// Wrapper for forwarding value
ClientLoggerStream& ClientLoggerStream::operator<<(bool value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(char value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(short value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(unsigned short value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(int value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(unsigned int value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(long value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(unsigned long value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(long long value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(unsigned long long value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(float value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(double value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(long double value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(const void* value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(const char* value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(const std::string& value) { *m_impl << value; return *this; }
ClientLoggerStream& ClientLoggerStream::operator<<(const std::filesystem::path& value) { *m_impl << value; return *this; }

ThreadSafeClientLoggerStream::ThreadSafeClientLoggerStream(ClientLogSeverityLevel level) :
	ClientLoggerStream(level)
{
}

void ThreadSafeClientLoggerStream::write_line()
{
	if (g_is_initialized)
	{
		std::lock_guard<std::mutex> lock(*g_logger_mutex);

		ClientLoggerStream::write_line();
	}
}