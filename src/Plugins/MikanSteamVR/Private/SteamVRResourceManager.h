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

	// Phase 1: load raw SteamVR data (background-thread safe)
	class SteamVRRenderModelResource* fetchRenderModel(const std::string& renderModelName);
	// Phase 2: create GL resources for any cached model that hasn't had them created yet (main thread only)
	void createPendingGraphicsResources();

private:
	class IMkGraphicsContext* m_ownerContext= nullptr;
	std::map<std::string, class SteamVRRenderModelResource*> m_renderModelCache;
};