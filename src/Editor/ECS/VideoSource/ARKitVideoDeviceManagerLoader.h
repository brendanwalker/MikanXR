#pragma once

#include "IARKitVideoDeviceManager.h"

#include <functional>
#include <future>
#include <string>
#include <vector>

// Async module-loading helper for the MikanARKitVideo plugin, mirroring
// NetworkVideoSourceSystem's load/startup/retry pattern (see
// NetworkVideoSourceSystem.h/.cpp in this same directory).
//
// This was originally (ticket B8) a standalone, manually-ticked scaffold named
// ARKitVideoSourceSystem, modeled on MikanServer's pattern and owned directly by
// MainWindow, since ARKitVideoSourceComponent/ARKitVideoSourceDefinition didn't
// exist yet and MikanTypedObjectSystem requires a working component/definition
// pair to even compile. Ticket E1 added those and folded the real
// ARKitVideoSourceSystem into a proper MikanTypedObjectSystem (owned by
// ProjectManager, like every other video source system) - this class was renamed
// to ARKitVideoDeviceManagerLoader and demoted to a plain composed helper that the
// new ARKitVideoSourceSystem owns, so its async-load/manager-ready-callback logic
// (already covered by arkit_video_source_system_unit_tests.cpp's B8-era test
// suite, which only needed to be retargeted at the new name) didn't need to be
// rewritten or lose coverage.
class ARKitVideoDeviceManagerLoader
{
public:
	ARKitVideoDeviceManagerLoader();
	~ARKitVideoDeviceManagerLoader();

	// Non-copyable
	ARKitVideoDeviceManagerLoader(const ARKitVideoDeviceManagerLoader&)= delete;
	ARKitVideoDeviceManagerLoader& operator=(const ARKitVideoDeviceManagerLoader&)= delete;

	void update(float deltaTime);
	void dispose();

	enum class eARKitVideoManagerState
	{
		uninitialized,
		initializing,
		ready,
		failed
	};
	eARKitVideoManagerState getARKitVideoManagerState() const { return m_arkitVideoManagerState; }
	bool isLoading() const { return m_arkitVideoManagerState == eARKitVideoManagerState::initializing; }

	IARKitVideoDeviceManagerPtr getARKitVideoDeviceManager() const { return m_arkitVideoDeviceManager; }

	// Fires `callback` once the device manager becomes ready - immediately
	// (synchronously) if it already is, otherwise once update() observes the
	// initializing -> ready transition. Never fires on failure, matching
	// NetworkVideoSourceSystem's retry loop, which likewise only runs on success -
	// a failed manager has nothing pending components could usefully retry.
	void addManagerReadyCallback(std::function<void()> callback);

protected:
	bool ensureARKitDeviceManager();

private:
	struct ARKitVideoDeviceManagerInitResult
	{
		class IARKitVideoDeviceModule* module= nullptr;
		IARKitVideoDeviceManagerPtr manager;
	};
	static ARKitVideoDeviceManagerInitResult initARKitVideoDeviceManagerOnThread(const std::string& moduleName);

	void disposeARKitVideoDeviceManager();

	eARKitVideoManagerState m_arkitVideoManagerState= eARKitVideoManagerState::uninitialized;
	std::future<ARKitVideoDeviceManagerInitResult> m_arkitVideoManagerFuture;
	class IARKitVideoDeviceModule* m_arkitVideoDeviceModule= nullptr;
	IARKitVideoDeviceManagerPtr m_arkitVideoDeviceManager= nullptr;

	std::vector<std::function<void()>> m_pendingManagerReadyCallbacks;
};
