#pragma once

#include <string>
#include <map>

class SteamVRResourceManager
{
public:
	SteamVRResourceManager();
	virtual ~SteamVRResourceManager();

	void init(class IMkWindow* ownerWindow);
	void cleanup();

	class SteamVRRenderModelResource* fetchRenderModel(const std::string &renderModelName);

private:
	class IMkWindow* m_ownerWindow= nullptr;
	std::map<std::string, class SteamVRRenderModelResource*> m_renderModelCache;
};
