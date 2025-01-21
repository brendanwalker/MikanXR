#pragma once

//-- utility methods -----
namespace ThreadUtils
{
	// Store the ID of the main thread
	void initMainThreadId();

	/// Returns true if the current thread is the main thread
	bool isRunningInMainThread();

	/// Sets the name of the current thread
	void setCurrentThreadName(const char* thread_name);

	/// Sleeps the current thread for the given number of milliseconds
	void sleepMilliseconds(int milliseconds);
};