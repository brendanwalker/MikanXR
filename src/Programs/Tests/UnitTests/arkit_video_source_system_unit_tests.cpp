//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <chrono>
#include <thread>

#include "ARKitVideoSourceSystem.h"
#include "MikanModuleManager.h"
#include "unit_test.h"

namespace
{
// ARKitVideoSourceSystem calls getMikanModuleManager(), which asserts non-null
// - the real app initializes this once at startup (see App.cpp); this is the
// first unit test module that needs it, so ensure it exists. Safe/idempotent
// to call more than once (see MikanModuleManager.cpp), and deliberately never
// shut down here - it's a process-lifetime singleton other test modules run
// after this one could also rely on existing.
void ensureModuleManagerInitialized()
{
	bool initialized= initMikanModuleManager();
	(void)initialized;
}

// Polls until `predicate` is true or the timeout elapses, ticking `system` each
// iteration - mirrors how a real per-frame update() loop would drive this
// class. Returns false on timeout.
template <typename Predicate>
bool pollUntil(ARKitVideoSourceSystem& system, Predicate predicate,
			   std::chrono::milliseconds timeout= std::chrono::seconds(10))
{
	const auto start= std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start < timeout)
	{
		system.update(0.016f);
		if (predicate())
			return true;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	return predicate();
}
} // namespace

//-- private functions -----
static bool arkit_video_source_system_test_initial_state()
{
	UNIT_TEST_BEGIN("default-constructed system starts uninitialized, no manager")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem system;
	success= (system.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::uninitialized);
	assert(success);
	success= success && !system.isLoading();
	assert(success);
	success= success && (system.getARKitVideoDeviceManager() == nullptr);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_source_system_test_missing_plugin_fails_gracefully_no_crash()
{
	UNIT_TEST_BEGIN("MikanARKitVideo.dll genuinely missing: async load fails gracefully, never crashes")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem system;

	// First update() should launch the async load (or, if GStreamer isn't
	// installed on this machine, fail synchronously via the cheap pre-check) -
	// either way it must not crash the process, which is this test's core
	// assertion: reaching the end of this function at all is proof of that.
	system.update(0.016f);
	success= (system.getARKitVideoManagerState() != ARKitVideoSourceSystem::eARKitVideoManagerState::uninitialized);
	assert(success);

	const bool reachedTerminalState=
		pollUntil(system,
				  [&]
				  {
					  const auto state= system.getARKitVideoManagerState();
					  return state == ARKitVideoSourceSystem::eARKitVideoManagerState::failed
							 || state == ARKitVideoSourceSystem::eARKitVideoManagerState::ready;
				  });
	success= success && reachedTerminalState;
	assert(success);

	// The plugin DLL does not exist on disk in this environment (Track C hasn't
	// built it yet), so regardless of whether GStreamer itself is installed, the
	// outcome must be "failed", never "ready".
	success= success && (system.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::failed);
	assert(success);
	success= success && (system.getARKitVideoDeviceManager() == nullptr);
	assert(success);
	success= success && !system.isLoading();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_source_system_test_repeated_update_while_initializing_is_safe()
{
	UNIT_TEST_BEGIN(
		"hammering update() while an async load is in flight doesn't spawn duplicate loads or corrupt state")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem system;

	// Tick many times in a tight loop, mimicking many frames landing before the
	// background thread has had a chance to resolve.
	for (int i= 0; i < 50; ++i)
		system.update(0.016f);

	const bool reachedFailed= pollUntil(
		system,
		[&] { return system.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::failed; });
	success= reachedFailed;
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_source_system_test_manager_ready_callback_never_fires_on_failure()
{
	UNIT_TEST_BEGIN("addManagerReadyCallback queues while pending and never fires on a failed load")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem system;

	int callbackCount= 0;
	system.addManagerReadyCallback([&] { ++callbackCount; });

	// Not yet ready - must not have fired synchronously.
	success= (callbackCount == 0);
	assert(success);

	const bool reachedFailed= pollUntil(
		system,
		[&] { return system.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::failed; });
	success= success && reachedFailed;
	assert(success);

	// A failed load must never fire the ready callback - there's nothing for a
	// waiting component to usefully retry.
	success= success && (callbackCount == 0);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_source_system_test_dispose_blocks_on_inflight_init_and_is_idempotent()
{
	UNIT_TEST_BEGIN("dispose() safely blocks on an in-flight async load and can be called more than once")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem system;
	system.update(0.016f); // launch the async load, don't wait for it

	// dispose() while init may still be in flight must not crash or hang forever.
	system.dispose();
	success= (system.getARKitVideoDeviceManager() == nullptr);
	assert(success);

	// Calling it again (and letting the destructor call it a third time) must
	// also be safe.
	system.dispose();
	success= success && (system.getARKitVideoDeviceManager() == nullptr);
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_source_system_test_independent_instances()
{
	UNIT_TEST_BEGIN("multiple instances track independent state against the shared module manager singleton")

	ensureModuleManagerInitialized();

	ARKitVideoSourceSystem systemA;
	ARKitVideoSourceSystem systemB;

	systemA.update(0.016f);
	success= (systemB.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::uninitialized);
	assert(success);

	const bool bothFailed= pollUntil(
		systemA,
		[&]
		{
			systemB.update(0.016f);
			return systemA.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::failed
				   && systemB.getARKitVideoManagerState() == ARKitVideoSourceSystem::eARKitVideoManagerState::failed;
		});
	success= success && bothFailed;
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_video_source_system_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_video_source_system")
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_initial_state);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_missing_plugin_fails_gracefully_no_crash);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_repeated_update_while_initializing_is_safe);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_manager_ready_callback_never_fires_on_failure);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_dispose_blocks_on_inflight_init_and_is_idempotent);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_source_system_test_independent_instances);
	UNIT_TEST_MODULE_END()
}
