#pragma once

// -- includes -----
#include "GStreamerCameraEnumerator.h"
#include "GStreamerVideoConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "OpenCVFwd.h"
#include "WorkerThread.h"
#include "VideoFwd.h"

// -- definitions -----
class GStreamerVideoFrameProcessor : public WorkerThread
{
public:
	GStreamerVideoFrameProcessor(const int deviceIndex, class IVideoSourceListener* listener);
	~GStreamerVideoFrameProcessor();

	bool openVideoMode(GStreamerVideoConfigPtr cfg);
	void closeVideoMode();
	bool getIsCameraOpen() const;

	bool startVideoFrameThread();
	void stopVideoFrameThread();
	bool getIsThreadRunning() const;

	//cv::VideoCapture* getVideoCaptureInterface() const { return m_videoCapture; }

protected:
	virtual bool doWork() override;

private:
	unsigned int m_deviceIndex;
	//cv::VideoCapture *m_videoCapture;
	cv::Mat *m_videoFrame;
	class IVideoSourceListener* m_videoSourceListener;
};

class GStreamerVideoDevice
{
public:
	GStreamerVideoDevice(const int deviceIndex, VideoCapabilitiesConfigConstPtr videoCaps);
	~GStreamerVideoDevice();

	bool open(GStreamerVideoConfigPtr cfg, class IVideoSourceListener* trackerListener);
	bool getIsOpen() const;
	void close();

	bool startVideoStream();
	bool getIsVideoStreaming();
	void stopVideoStream();

	//inline const VideoPropertyConstraint* getVideoPropertyConstraints() const { return m_videoPropertyConstraints; }
	inline const GStreamerVideoFrameProcessor* getVideoFrameProcessorConst() const { return m_videoFrameProcessor; }
	inline GStreamerVideoFrameProcessor* getVideoFrameProcessor() { return m_videoFrameProcessor; }
	const VideoModeConfig* getCurrentVideoMode() const;

	bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const;
	void setVideoProperty(const VideoPropertyType property_type, int desired_value);
	int getVideoProperty(const VideoPropertyType property_type) const;

public:
	int m_deviceIndex;
	//VideoCapabilitiesConfigConstPtr m_videoCapabilities;
	//int m_deviceModeIndex;
	//VideoPropertyConstraint m_videoPropertyConstraints[(int)VideoPropertyType::COUNT];
	GStreamerVideoFrameProcessor* m_videoFrameProcessor;
};
