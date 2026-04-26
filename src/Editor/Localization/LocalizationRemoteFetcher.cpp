#include "LocalizationRemoteFetcher.h"
#include "Logger.h"

#include <fstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244) // conversion from X to Y, possible loss of data
#endif
#include <nlohmann/json.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

LocalizationRemoteFetcher::LocalizationRemoteFetcher(
	const std::string& baseUrl,
	const std::filesystem::path& cacheDir,
	std::chrono::hours cacheTTL)
	: m_baseUrl(baseUrl)
	, m_cacheDir(cacheDir)
	, m_cacheTTL(cacheTTL)
{
}

LocalizationRemoteFetcher::~LocalizationRemoteFetcher()
{
	cancelFetch();
}

void LocalizationRemoteFetcher::startFetch(std::function<void(bool)> onComplete)
{
	if (m_thread.joinable())
		return;

	m_cancelled = false;
	m_onComplete = onComplete;
	m_thread = std::thread(&LocalizationRemoteFetcher::fetchThread, this);
}

void LocalizationRemoteFetcher::cancelFetch()
{
	m_cancelled = true;
	if (m_thread.joinable())
		m_thread.join();
}

bool LocalizationRemoteFetcher::isCacheFresh() const
{
	const std::filesystem::path manifestPath = m_cacheDir / "manifest.json";
	if (!std::filesystem::exists(manifestPath))
		return false;

	std::error_code ec;
	const auto mtime = std::filesystem::last_write_time(manifestPath, ec);
	if (ec)
		return false;

	const auto age = std::filesystem::file_time_type::clock::now() - mtime;
	return age < m_cacheTTL;
}

bool LocalizationRemoteFetcher::httpGet(const std::string& url, std::string& outBody)
{
	outBody = "";

#ifdef _WIN32
	// Expect "https://host/path"
	const std::string httpsPrefix = "https://";
	if (url.substr(0, httpsPrefix.size()) != httpsPrefix)
	{
		MIKAN_LOG_ERROR("LocalizationRemoteFetcher::httpGet") << "Only HTTPS URLs are supported: " << url;
		return false;
	}

	const std::string remainder = url.substr(httpsPrefix.size());
	const auto slashPos = remainder.find('/');
	const std::string hostStr = (slashPos != std::string::npos) ? remainder.substr(0, slashPos) : remainder;
	const std::string pathStr = (slashPos != std::string::npos) ? remainder.substr(slashPos) : "/";

	const std::wstring host(hostStr.begin(), hostStr.end());
	const std::wstring path(pathStr.begin(), pathStr.end());

	bool success = false;

	HINTERNET hSession = WinHttpOpen(
		L"MikanXR/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (!hSession)
	{
		MIKAN_LOG_WARNING("LocalizationRemoteFetcher::httpGet") << "WinHttpOpen failed (" << GetLastError() << ")";
		return false;
	}

	// Resolve/connect: 10s, send: 10s, receive: 30s
	WinHttpSetTimeouts(hSession, 10000, 10000, 10000, 30000);

	HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (hConnect)
	{
		HINTERNET hRequest = WinHttpOpenRequest(
			hConnect,
			L"GET",
			path.c_str(),
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);
		if (hRequest)
		{
			if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0) &&
				WinHttpReceiveResponse(hRequest, NULL))
			{
				DWORD statusCode = 0;
				DWORD statusCodeSize = sizeof(statusCode);
				WinHttpQueryHeaders(
					hRequest,
					WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
					WINHTTP_HEADER_NAME_BY_INDEX,
					&statusCode, &statusCodeSize,
					WINHTTP_NO_HEADER_INDEX);

				if (statusCode == 200)
				{
					DWORD bytesAvailable = 0;
					while (!m_cancelled &&
						WinHttpQueryDataAvailable(hRequest, &bytesAvailable) &&
						bytesAvailable > 0)
					{
						std::vector<char> buffer(bytesAvailable);
						DWORD bytesRead = 0;
						if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead))
							outBody.append(buffer.data(), bytesRead);
					}
					success = !m_cancelled;
				}
				else
				{
					MIKAN_LOG_WARNING("LocalizationRemoteFetcher::httpGet")
						<< "HTTP " << statusCode << " for " << url;
				}
			}
			else
			{
				MIKAN_LOG_WARNING("LocalizationRemoteFetcher::httpGet")
					<< "WinHttp send/receive failed (" << GetLastError() << ") for " << url;
			}

			WinHttpCloseHandle(hRequest);
		}
		WinHttpCloseHandle(hConnect);
	}

	WinHttpCloseHandle(hSession);
	return success;
#else
	MIKAN_LOG_ERROR("LocalizationRemoteFetcher::httpGet") << "Not supported on this platform";
	return false;
#endif // _WIN32
}

bool LocalizationRemoteFetcher::fetchManifest(std::vector<std::string>& outFiles)
{
	const std::string manifestUrl = m_baseUrl + "/manifest.json";
	std::string body;
	if (!httpGet(manifestUrl, body))
		return false;

	// Write manifest to cache so its mtime tracks freshness
	const std::filesystem::path manifestPath = m_cacheDir / "manifest.json";
	{
		std::ofstream fileStream(manifestPath, std::ios::binary);

		if (fileStream)
		{
			fileStream << body;
		}
	}

	try
	{
		const nlohmann::json j = nlohmann::json::parse(body);
		for (const auto& file : j.at("files"))
			outFiles.push_back(file.get<std::string>());

		return true;
	}
	catch (const std::exception& e)
	{
		MIKAN_LOG_ERROR("LocalizationRemoteFetcher::fetchManifest")
			<< "Failed to parse manifest JSON: " << e.what();

		return false;
	}
}

bool LocalizationRemoteFetcher::fetchFile(const std::string& filename)
{
	const std::string fileUrl = m_baseUrl + "/" + filename;
	std::string body;
	if (!httpGet(fileUrl, body))
		return false;

	const std::filesystem::path filePath = m_cacheDir / filename;
	std::ofstream out(filePath, std::ios::binary);
	if (!out)
	{
		MIKAN_LOG_ERROR("LocalizationRemoteFetcher::fetchFile")
			<< "Failed to write cache file: " << filePath;
		return false;
	}
	out << body;
	return true;
}

void LocalizationRemoteFetcher::fetchThread()
{
	if (isCacheFresh())
	{
		MIKAN_LOG_INFO("LocalizationRemoteFetcher") << "Localization cache is fresh, skipping remote fetch.";
		if (m_onComplete)
			m_onComplete(true);
		return;
	}

	// Ensure cache directory exists
	std::error_code ec;
	std::filesystem::create_directories(m_cacheDir, ec);
	if (ec)
	{
		MIKAN_LOG_ERROR("LocalizationRemoteFetcher") << "Failed to create cache directory: " << ec.message();
		if (m_onComplete)
			m_onComplete(false);
		return;
	}

	// Fetch manifest (also writes it to cache to update mtime)
	std::vector<std::string> files;
	if (!fetchManifest(files))
	{
		MIKAN_LOG_WARNING("LocalizationRemoteFetcher") << "Failed to fetch localization manifest from " << m_baseUrl;
		if (m_onComplete)
			m_onComplete(false);
		return;
	}

	MIKAN_LOG_INFO("LocalizationRemoteFetcher") << "Fetching " << files.size() << " localization file(s) from CDN...";

	bool allSucceeded = true;
	for (const auto& filename : files)
	{
		if (m_cancelled)
			break;

		if (!fetchFile(filename))
		{
			MIKAN_LOG_WARNING("LocalizationRemoteFetcher") << "Failed to fetch: " << filename;
			allSucceeded = false;
		}
		else
		{
			MIKAN_LOG_INFO("LocalizationRemoteFetcher") << "Cached: " << filename;
		}
	}

	if (m_onComplete)
		m_onComplete(allSucceeded && !m_cancelled);
}
