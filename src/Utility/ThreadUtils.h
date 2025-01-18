#pragma once

//-- utility methods -----
namespace ThreadUtils
{
	/// Sets the name of the current thread
	void setCurrentThreadName(const char* thread_name);

	/// Sleeps the current thread for the given number of milliseconds
	void sleepMilliseconds(int milliseconds);
};