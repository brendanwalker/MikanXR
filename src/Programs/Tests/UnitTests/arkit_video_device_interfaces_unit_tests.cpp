//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "IARKitVideoDeviceModule.h"
#include "unit_test.h"

// This ticket is a pure interface definition (IARKitVideoDevice/Manager/Module,
// mirroring INetworkVideoDevice/Manager/Module) - there's no runtime behavior of
// its own to test yet. What's verified here is that the interfaces are genuinely
// implementable and callable through the base-class pointers (correct signatures,
// no accidental pure-virtual left unimplemented, listener dispatch works, the
// module->manager->device chain wires together) - a stronger check than a bare
// #include-only compile smoke test, using minimal no-op stub implementations.
namespace
{
class StubARKitVideoDevice : public IARKitVideoDevice
{
public:
	// IVideoDevice
	const char* getDevicePath() const override { return m_devicePath.c_str(); }
	const char* getFriendlyName() const override { return "Stub ARKit Device"; }
	bool isVideoSettingSupported(const eVideoSettingType) const override { return false; }
	bool getVideoSettingConstraint(const eVideoSettingType, VideoSettingConstraint&) const override { return false; }
	void setVideoSetting(const eVideoSettingType, int) override {}
	int getVideoSetting(const eVideoSettingType) const override { return 0; }

	// IARKitVideoDevice
	void update(float) override {}

	void addListener(IARKitVideoDeviceListener* listener) override { m_listener= listener; }
	void removeListener(IARKitVideoDeviceListener* listener) override
	{
		if (m_listener == listener)
			m_listener= nullptr;
	}

	eVideoOpeningStatus getVideoOpeningStatus() const override { return m_openingStatus; }
	eVideoOpeningStatus open() override
	{
		m_openingStatus= eVideoOpeningStatus::open;
		if (m_listener)
			m_listener->notifyDeviceOpened(this);
		return m_openingStatus;
	}
	void close() override
	{
		m_openingStatus= eVideoOpeningStatus::closed;
		if (m_listener)
			m_listener->notifyDeviceClosed(this);
	}

	eVideoStreamingStatus startVideoStream() override
	{
		m_streamingStatus= eVideoStreamingStatus::started;
		return m_streamingStatus;
	}
	eVideoStreamingStatus getVideoStreamingStatus() const override { return m_streamingStatus; }
	void stopVideoStream() override { m_streamingStatus= eVideoStreamingStatus::stopped; }

	// Zero-copy CUDA-GL texture access (ticket E3) - this stub never has a real GL
	// texture, 0 is the documented "none yet" sentinel.
	uint32_t getColorTextureGlId() const override { return 0; }
	uint32_t getDepthTextureGlId() const override { return 0; }

	// Test-only helper to exercise the listener dispatch path.
	void simulateFrameBundle(const ARKitVideoFrameBundle& bundle)
	{
		if (m_listener)
			m_listener->notifyFrameBundleReceived(bundle);
	}

	void setDevicePath(const std::string& path) { m_devicePath= path; }

private:
	std::string m_devicePath= "arkit://test";
	IARKitVideoDeviceListener* m_listener= nullptr;
	eVideoOpeningStatus m_openingStatus= eVideoOpeningStatus::closed;
	eVideoStreamingStatus m_streamingStatus= eVideoStreamingStatus::stopped;
};

class StubARKitVideoDeviceManager : public IARKitVideoDeviceManager
{
public:
	bool startup() override
	{
		m_startedUp= true;
		return true;
	}
	void update(float) override { ++m_updateCount; }
	void shutdown() override
	{
		m_devices.clear();
		m_startedUp= false;
	}

	size_t getDeviceCount() const override { return m_devices.size(); }
	IARKitVideoDevicePtr getDeviceByIndex(size_t index) override
	{
		return index < m_devices.size() ? m_devices[index] : nullptr;
	}
	IARKitVideoDevicePtr getDeviceByPath(const char* devicePath) override
	{
		for (auto& device : m_devices)
		{
			if (std::strcmp(device->getDevicePath(), devicePath) == 0)
				return device;
		}
		return nullptr;
	}

	IARKitVideoDevicePtr createVideoDevice(const ARKitVideoConnectionSettings& settings) override
	{
		auto device= std::make_shared<StubARKitVideoDevice>();
		device->setDevicePath("arkit://" + std::to_string(settings.basePort));
		m_devices.push_back(device);
		return device;
	}
	void destroyVideoDevice(IARKitVideoDevicePtr device) override
	{
		m_devices.erase(std::remove(m_devices.begin(), m_devices.end(), device), m_devices.end());
	}

	bool wasStartedUp() const { return m_startedUp; }
	int getUpdateCount() const { return m_updateCount; }

private:
	std::vector<IARKitVideoDevicePtr> m_devices;
	bool m_startedUp= false;
	int m_updateCount= 0;
};

class StubARKitVideoDeviceModule : public IARKitVideoDeviceModule
{
public:
	bool startup() override { return true; }
	void shutdown() override {}

	IARKitVideoDeviceManagerPtr createARKitVideoDeviceManager() override
	{
		return std::make_shared<StubARKitVideoDeviceManager>();
	}
};

// Only overrides the two pure-virtual listener methods, deliberately leaving
// notifyDeviceOpened at its default (non-pure) empty implementation - proves
// that method is genuinely optional, mirroring
// INetworkVideoDeviceListener::notifyVideoDeviceOpened.
class RecordingListener : public IARKitVideoDeviceListener
{
public:
	int closedCount= 0;
	int bundleCount= 0;
	ARKitVideoFrameBundle lastBundle;

	void notifyDeviceClosed(const IARKitVideoDevice*) override { ++closedCount; }
	void notifyFrameBundleReceived(const ARKitVideoFrameBundle& bundle) override
	{
		++bundleCount;
		lastBundle= bundle;
	}
};
} // namespace

//-- private functions -----
static bool arkit_video_device_interfaces_test_module_manager_device_chain()
{
	UNIT_TEST_BEGIN("module -> manager -> device chain wires together and is callable")

	StubARKitVideoDeviceModule module;
	success= module.startup();
	assert(success);

	IARKitVideoDeviceManagerPtr manager= module.createARKitVideoDeviceManager();
	success= success && (manager != nullptr);
	assert(success);

	success= success && manager->startup();
	assert(success);

	ARKitVideoConnectionSettings settings;
	settings.basePort= 27100;
	settings.depthStreamingEnabled= true;

	IARKitVideoDevicePtr device= manager->createVideoDevice(settings);
	success= success && (device != nullptr);
	assert(success);
	success= success && (manager->getDeviceCount() == 1);
	assert(success);
	success= success && (manager->getDeviceByIndex(0) == device);
	assert(success);
	success= success && (manager->getDeviceByPath(device->getDevicePath()) == device);
	assert(success);

	success= success && (device->getVideoOpeningStatus() == eVideoOpeningStatus::closed);
	assert(success);
	success= success && (device->open() == eVideoOpeningStatus::open);
	assert(success);
	success= success && (device->getVideoOpeningStatus() == eVideoOpeningStatus::open);
	assert(success);

	success= success && (device->startVideoStream() == eVideoStreamingStatus::started);
	assert(success);
	device->stopVideoStream();
	success= success && (device->getVideoStreamingStatus() == eVideoStreamingStatus::stopped);
	assert(success);

	device->close();
	success= success && (device->getVideoOpeningStatus() == eVideoOpeningStatus::closed);
	assert(success);

	manager->destroyVideoDevice(device);
	success= success && (manager->getDeviceCount() == 0);
	assert(success);

	manager->shutdown();
	module.shutdown();

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_device_interfaces_test_manager_update_and_startup_tracking()
{
	UNIT_TEST_BEGIN("manager startup/update/shutdown lifecycle is distinct from per-device open/close")

	StubARKitVideoDeviceManager manager;
	success= !manager.wasStartedUp();
	assert(success);

	success= success && manager.startup();
	assert(success);
	success= success && manager.wasStartedUp();
	assert(success);

	manager.update(0.016f);
	manager.update(0.016f);
	success= success && (manager.getUpdateCount() == 2);
	assert(success);

	manager.shutdown();
	success= success && !manager.wasStartedUp();
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_device_interfaces_test_listener_dispatch()
{
	UNIT_TEST_BEGIN("listener receives open/close/frame-bundle notifications through the base interface")

	StubARKitVideoDevice device;
	RecordingListener listener;
	device.addListener(&listener);

	device.open();
	success=
		(listener.closedCount == 0); // notifyDeviceOpened has no counter (default no-op override) - just must not crash
	assert(success);

	ARKitVideoFrameBundle bundle;
	bundle.frameSeq= 42;
	bundle.timestampUs= 123456789ULL;
	bundle.hasVideo= true;
	bundle.videoWidth= 1920;
	bundle.videoHeight= 1080;
	bundle.hasPose= true;
	bundle.pose.fx= 1428.5f;
	bundle.hasDepth= true;
	bundle.depth.width= 256;
	bundle.depth.height= 192;

	device.simulateFrameBundle(bundle);
	success= success && (listener.bundleCount == 1);
	assert(success);
	success= success && (listener.lastBundle.frameSeq == 42);
	success= success && (listener.lastBundle.timestampUs == 123456789ULL);
	success= success && (listener.lastBundle.hasVideo && listener.lastBundle.videoWidth == 1920);
	success= success && (listener.lastBundle.hasPose && listener.lastBundle.pose.fx == 1428.5f);
	success= success && (listener.lastBundle.hasDepth && listener.lastBundle.depth.width == 256);
	assert(success);

	device.close();
	success= success && (listener.closedCount == 1);
	assert(success);

	device.removeListener(&listener);
	device.simulateFrameBundle(bundle);
	success= success && (listener.bundleCount == 1); // unchanged - listener was removed
	assert(success);

	UNIT_TEST_COMPLETE()
}

static bool arkit_video_device_interfaces_test_frame_bundle_default_state()
{
	UNIT_TEST_BEGIN("ARKitVideoFrameBundle/ARKitDepthFrameBuffer/ARKitPoseFrameBuffer default-construct cleanly")

	ARKitVideoFrameBundle bundle;
	success= (bundle.frameSeq == 0 && bundle.timestampUs == 0);
	assert(success);
	success= success && (!bundle.hasVideo && bundle.videoData == nullptr && bundle.videoWidth == 0);
	assert(success);
	success= success && (!bundle.hasDepth && bundle.depth.width == 0 && bundle.depth.depthMM == nullptr);
	assert(success);
	success= success && (!bundle.hasPose && bundle.pose.fx == 0.f);
	assert(success);
	for (int i= 0; success && i < 16; ++i)
		success= (bundle.pose.transform[i] == 0.f);
	assert(success);

	UNIT_TEST_COMPLETE()
}

//-- public interface -----
bool run_arkit_video_device_interfaces_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("arkit_video_device_interfaces")
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_device_interfaces_test_module_manager_device_chain);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_device_interfaces_test_manager_update_and_startup_tracking);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_device_interfaces_test_listener_dispatch);
	UNIT_TEST_MODULE_CALL_TEST(arkit_video_device_interfaces_test_frame_bundle_default_state);
	UNIT_TEST_MODULE_END()
}
