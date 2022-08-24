#pragma once

#include <mutex>
#include <string>
#include <stdint.h>

#include "WorkerThread.h"

// Adapted from https://github.com/dataplayer12/video-writer
class VideoWriter : public WorkerThread
{
public:
	VideoWriter();
	~VideoWriter();

	bool open(const std::string& filename, float fps, int fwidth, int fheight, int bpp);
	void close();

	bool write(class GlTexture* bgrTexture);

protected:
	virtual bool doWork() override;

private:
	std::string m_filepath;
	float m_fps;
	int m_frameWidth;
	int m_frameHeight;
	int m_bitsPerPixel;

	bool allocateFrameBuffers();
	void freeFrameBuffers();
	bool setupEncoder();
	bool addVideoStream();
	bool getIsReadyToWrite();

	uint64_t m_presentationTimestamp = 1;
	struct AVIOContext* m_avioContext = nullptr;
	struct AVCodecContext* m_avCodecContext = nullptr;
	struct AVFormatContext* m_avFormatContext = nullptr;
	struct AVStream* m_avStream = nullptr;
	struct AVFrame* m_avFrame = nullptr;
	struct AVCodec* m_codecEncoder = nullptr;
	struct SwsContext* m_bgrToYUV420pConverter = nullptr;

	std::mutex m_frameMutex;
	std::condition_variable m_frameConsumerWait;
	uint8_t** m_bgrFrameCirularBuffer= nullptr;
	size_t m_bgrFrameSizeBytes= 0;
	uint32_t m_frameCirularBufferSize= 0;
	uint32_t m_frameReadIndex = 0;
	uint32_t m_frameWriteIndex = 0;
};