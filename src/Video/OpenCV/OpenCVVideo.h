#pragma once

#pragma once

// -- includes -----
#include "OpenCVCameraEnumerator.h"
#include "OpenCVVideoConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "WorkerThread.h"

namespace cv
{
	class VideoCapture;
	class Mat;
};

// -- definitions -----
class OpenCVVideoFrameProcessor : public WorkerThread
{
public:
	OpenCVVideoFrameProcessor(const int deviceIndex, class IVideoSourceListener* listener);
	~OpenCVVideoFrameProcessor();

	bool openVideoMode(const VideoModeConfig& videoModeConfig);
	void closeVideoMode();
	bool getIsCameraOpen() const;

	bool startVideoFrameThread();
	void stopVideoFrameThread();
	bool getIsThreadRunning() const;

	cv::VideoCapture* getVideoCaptureInterface() const { return m_videoCapture; }

protected:
	virtual bool doWork() override;

private:
	unsigned int m_deviceIndex;
	cv::VideoCapture *m_videoCapture;
	cv::Mat *m_videoFrame;
	class IVideoSourceListener* m_videoSourceListener;
};

class OpenCVVideoDevice
{
public:
	OpenCVVideoDevice(const int deviceIndex, const VideoCapabilitiesConfig &videoCaps);
	~OpenCVVideoDevice();

	bool open(int desiredModeIndex, OpenCVVideoConfig& cfg, class IVideoSourceListener* trackerListener);
	bool getIsOpen() const;
	void close();

	bool startVideoStream();
	bool getIsVideoStreaming();
	void stopVideoStream();

	inline const VideoPropertyConstraint* getVideoPropertyConstraints() const { return m_videoPropertyConstraints; }
	inline const OpenCVVideoFrameProcessor* getVideoFrameProcessorConst() const { return m_videoFrameProcessor; }
	inline OpenCVVideoFrameProcessor* getVideoFrameProcessor() { return m_videoFrameProcessor; }
	const VideoModeConfig* getCurrentVideoMode() const;

	bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const;
	void setVideoProperty(const VideoPropertyType property_type, int desired_value);
	int getVideoProperty(const VideoPropertyType property_type) const;

public:
	int m_deviceIndex;
	VideoCapabilitiesConfig m_videoCapabilities;
	int m_deviceModeIndex;
	VideoPropertyConstraint m_videoPropertyConstraints[(int)VideoPropertyType::COUNT];
	OpenCVVideoFrameProcessor* m_videoFrameProcessor;
};
