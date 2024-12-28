#pragma once

#include <string>
#include <map>

class SteamVRResourceManager
{
public:
	SteamVRResourceManager();
	virtual ~SteamVRResourceManager();

	void init(class IGlWindow* ownerWindow);
	void cleanup();

	class SteamVRRenderModelResource* fetchRenderModel(const std::string &renderModelName);

private:
	class IGlWindow* m_ownerWindow= nullptr;
	std::map<std::string, class SteamVRRenderModelResource*> m_renderModelCache;
};
