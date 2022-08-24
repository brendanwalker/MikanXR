#pragma once

#include <string>
#include <map>

class SteamVRResourceManager
{
public:
	SteamVRResourceManager();
	virtual ~SteamVRResourceManager();

	void init();
	void cleanup();

	class SteamVRRenderModelResource* fetchRenderModel(const std::string &renderModelName);

private:
	std::map<std::string, class SteamVRRenderModelResource*> m_renderModelCache;
};
