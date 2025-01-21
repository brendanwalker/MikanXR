//-- includes -----
#include "Logger.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <ostream>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': localtime
#endif

#ifdef WIN32
#include <windows.h>
#endif

//-- globals -----
bool g_is_initialized= false;
LogSeverityLevel g_min_log_level= LogSeverityLevel::info;
bool g_is_console_log_enabled= false;
std::ostream* g_file_stream = nullptr;
std::mutex* g_logger_mutex = nullptr;
t_logCallback g_logger_callback = nullptr;

void log_default_callback(int log_level, const char* line)
{
	if (g_is_console_log_enabled)
	{
		if (log_level >= (int)LogSeverityLevel::error)
			std::cerr << line << std::endl;
		else
			std::cout << line << std::endl;
	}

	if (g_file_stream != nullptr)
	{
		*g_file_stream << line << std::endl;
	}
}

//-- public implementation -----
#ifdef WIN32
bool RedirectIOToConsole()
{
	if (GetConsoleWindow() == NULL)
	{
		if (!AllocConsole())
		{
			return false;
		}
	}

	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	return true;
}
#endif // WIN32

void log_init(const LoggerSettings& settings)
{
	if (!g_is_initialized)
	{
		g_min_log_level = settings.min_log_level;

		if (settings.enable_console)
		{
#ifdef WIN32
			if (RedirectIOToConsole())
			{
				g_is_console_log_enabled = true;
			}
#else
			g_is_console_log_enabled = true;
#endif			
		}

		if (settings.log_filename.length() > 0)
		{
			g_file_stream = new std::ofstream(settings.log_filename, std::ofstream::out);
		}

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

void log_dispose()
{
	if (g_file_stream != nullptr)
	{
		delete g_file_stream;
		g_file_stream = nullptr;
	}

	if (g_logger_mutex != nullptr)
	{
		delete g_logger_mutex;
		g_logger_mutex = nullptr;
	}

	if (g_is_console_log_enabled)
	{
#ifdef WIN32
		FreeConsole();
#endif
		g_is_console_log_enabled = false;
	}

	g_is_initialized = false;
}

bool log_can_emit_level(LogSeverityLevel level)
{
    return (level >= g_min_log_level);
}

std::string log_get_timestamp_prefix()
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
LoggerStream::LoggerStream(LogSeverityLevel level) :
	m_level(level)
{
}

LoggerStream::~LoggerStream()
{
	write_line();
}

void LoggerStream::write_line()
{
	if (g_is_initialized && 
		g_logger_callback != nullptr && 
		m_hasWrittenLog &&
		log_can_emit_level(m_level))
	{
		const std::string line = m_lineBuffer.str();

		(*g_logger_callback)((int)m_level, line.c_str());
	}
}

ThreadSafeLoggerStream::ThreadSafeLoggerStream(LogSeverityLevel level) :
	LoggerStream(level)
{
}

void ThreadSafeLoggerStream::write_line()
{
	if (g_is_initialized)
	{
		std::lock_guard<std::mutex> lock(*g_logger_mutex);

		LoggerStream::write_line();
	}
}