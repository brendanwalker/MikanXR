#pragma once

#include "IARKitVideoDeviceManager.h"

#include <functional>
#include <future>
#include <string>
#include <vector>

// Async module-loading scaffold for the MikanARKitVideo plugin, mirroring
// NetworkVideoSourceSystem's load/startup/retry pattern (see
// NetworkVideoSourceSystem.h/.cpp in this same directory).
//
// Unlike NetworkVideoSourceSystem, this is NOT (yet) a MikanTypedObjectSystem: that
// template requires a fully-built ECS component/definition pair
// (ARKitVideoSourceComponent/ARKitVideoSourceDefinition, ticket E1) with working
// constructors and JSON serialization before it will even compile correctly, let
// alone run safely (see MikanTypedObjectSystem.h's objectFactory()). Since E1
// hasn't landed yet, this class is instead a plain, manually-ticked manager class,
// modeled on MikanServer's pattern - MikanServer/ClientSourceManager/InputManager/
// OpenCVManager are all owned as plain members of MainWindow and ticked directly
// from MainWindow::update(), entirely bypassing ProjectManager/MikanObjectSystem
// (see MainWindow.h/.cpp).
//
// Once ticket E1 lands, this scaffold's async-load/manager-ready-callback pattern
// is what a real ARKitVideoSourceComponent should hook into to retry a deferred
// openVideoSource() call: addManagerReadyCallback() is the component-agnostic
// stand-in for NetworkVideoSourceSystem's direct "retry openVideoSource() on
// pending components" component-map walk, since there's no component map here yet.
class ARKitVideoSourceSystem
{
public:
	ARKitVideoSourceSystem();
	~ARKitVideoSourceSystem();

	// Non-copyable
	ARKitVideoSourceSystem(const ARKitVideoSourceSystem&)= delete;
	ARKitVideoSourceSystem& operator=(const ARKitVideoSourceSystem&)= delete;

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
