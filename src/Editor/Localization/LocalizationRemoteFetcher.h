#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <vector>

// Fetches localization CSV files from a remote CDN into a local cache directory.
// The fetch runs on a background thread and does not block app startup.
// Cache freshness is checked against a TTL; stale or absent cache triggers a re-fetch.
// On non-Windows platforms this class is a no-op (WinHTTP is used for HTTPS).
class LocalizationRemoteFetcher
{
public:
	LocalizationRemoteFetcher(const std::string& baseUrl, const std::filesystem::path& cacheDir,
							  std::chrono::hours cacheTTL= std::chrono::hours(24));
	~LocalizationRemoteFetcher();

	// Starts a background fetch. Optional callback fires when the fetch completes.
	// Safe to call only once; subsequent calls are ignored if already running.
	void startFetch(std::function<void(bool)> onComplete= nullptr);

	// Signals the background thread to stop and blocks until it exits.
	void cancelFetch();

private:
	void fetchThread();
	bool isCacheFresh() const;
	bool fetchManifest(std::vector<std::string>& outFiles);
	bool fetchFile(const std::string& filename);

#ifdef _WIN32
	// Performs a synchronous HTTPS GET using WinHTTP.
	bool httpGet(const std::string& url, std::string& outBody);
#endif

	std::string m_baseUrl;
	std::filesystem::path m_cacheDir;
	std::chrono::hours m_cacheTTL;
	std::thread m_thread;
	std::atomic<bool> m_cancelled{false};
	std::function<void(bool)> m_onComplete;
};
