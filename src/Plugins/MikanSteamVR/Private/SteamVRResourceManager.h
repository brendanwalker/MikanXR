#pragma once

#include <string>
#include <map>

class SteamVRResourceManager
{
public:
	SteamVRResourceManager();
	virtual ~SteamVRResourceManager();

	void init(class IMkGraphicsContext* ownerWindow);
	void cleanup();

	class SteamVRRenderModelResource* fetchRenderModel(const std::string& renderModelName);

private:
	class IMkGraphicsContext* m_ownerContext = nullptr;
	std::map<std::string, class SteamVRRenderModelResource*> m_renderModelCache;
};