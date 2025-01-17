#pragma once

#include "MikanGStreamerVideoInterface.h"

class MikanGStreamerVideoDevice : public IMikanGStreamerVideoDevice
{
public:
	MikanGStreamerVideoDevice(const MikanGStreamerSettings& settings);
	virtual ~MikanGStreamerVideoDevice();
	
	// Create / Destroy a GStreamer Video Pipeline based on the settings
	bool open() override;
	bool getIsOpen() const override;
	void close() override;

	// Start / Stop an opened pipeline
	bool startVideoStream() override;
	bool getIsVideoStreaming() const override;
	void stopVideoStream() override;

	// Try and fetch the next video frame from a started pipeline
	void tryPullSample(
		const MikanGStreamerVideoMode& inVideoMode,
		void (*onVideoModeChanged)(const MikanGStreamerVideoMode& newVideoMode, void* userdata),
		void (*onVideoFrameReceived)(const MikanGStreamerBuffer& newBuffer, void* userdata),
		void* userdata) override;

private:
	struct GStreamerImpl* m_impl;
	bool m_bIsStreaming;
};
