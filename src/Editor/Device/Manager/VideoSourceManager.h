#pragma once

//-- includes -----
#include <memory>
#include <deque>
#include <vector>

#include "CommonConfig.h"
#include "DeviceManager.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"

//-- typedefs -----
class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;
typedef std::vector<VideoSourceViewPtr> VideoSourceList;

//-- definitions -----
class VideoSourceManagerConfig : public CommonConfig
{
public:
	VideoSourceManagerConfig(const std::string& fnamebase = "VideoSourceManagerConfig");

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::vector<std::string> videoSourceURIs;
};

class VideoSourceManager : public DeviceManager
{
public:
	VideoSourceManager();

	inline static VideoSourceManager* getInstance() { return m_instance; }
	const VideoSourceManagerConfig& getConfig() const { return m_cfg; }

	class IMikanGStreamerModule* getGStreamerModule() const;

	bool startup(class IMkWindow *ownerWindow) override;
	void update(float deltaTime) override;
	void shutdown() override;

	void closeAllVideoSources();

	static const int k_max_devices = 8;
	int getMaxDevices() const override
	{
		return VideoSourceManager::k_max_devices;
	}

	VideoSourceViewPtr getVideoSourceViewPtr(int device_id) const;
	VideoSourceViewPtr getVideoSourceViewByPath(const std::string& devicePath) const;
	VideoSourceList getVideoSourceList() const;

protected:
	DeviceEnumerator* allocateDeviceEnumerator() override;
	void freeDeviceEnumerator(DeviceEnumerator*) override;
	DeviceView* allocateDeviceView(int device_id) override;

private:
	static VideoSourceManager* m_instance;

	class IMikanGStreamerModule* m_mikanGStreamerModule;
	VideoSourceManagerConfig m_cfg;
	class VideoCapabilitiesSet* m_supportedTrackers;
};

class VideoSourceListIterator
{
public:
	VideoSourceListIterator()= default;
	VideoSourceListIterator(const std::string &usbPath);

	inline bool hasVideoSources() const { return m_videoSourceList.size() > 0; }
	inline int getCurrentIndex() const { return m_listIndex; }
	VideoSourceViewPtr getCurrent() const;
	bool goPrevious();
	bool goNext();
	bool goToIndex(int new_index);

private:
	VideoSourceList m_videoSourceList;
	int m_listIndex= 0;
};