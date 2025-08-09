#include "OpenCVVideoFrameBuffer.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include <easy/profiler.h>

OpenCVVideoFrameBuffer::OpenCVVideoFrameBuffer(
	int bufferPixelWidth, int bufferPixelHeight,
	int framePixelWidth, int framePixelHeight,
	VideoFrameSection _section)
	: m_section(_section)
	, m_bgrBuffer(nullptr)
	, m_lastVideoFrameWriteIndex(0)
{
	m_srcBufferWidth = bufferPixelWidth;
	m_srcBufferHeight = bufferPixelHeight;
	m_frameWidth = framePixelWidth;
	m_frameHeight = framePixelHeight;

	m_bgrBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
}

OpenCVVideoFrameBuffer::~OpenCVVideoFrameBuffer()
{
	if (m_bgrBuffer != nullptr)
	{
		delete m_bgrBuffer;
	}
}

void OpenCVVideoFrameBuffer::writeVideoFrame(
	const unsigned char* video_buffer, 
	bool bIsFlipped)
{
	EASY_FUNCTION();

	std::lock_guard<std::mutex> bufferLock(m_bufferMutex);
	const cv::Mat videoBufferMat(
		m_srcBufferHeight, m_srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(video_buffer));

	if (bIsFlipped)
	{
		cv::flip(videoBufferMat, *m_bgrBuffer, +1);
	}
	else
	{
		videoBufferMat.copyTo(*m_bgrBuffer);
	}

	// Atomically increment the frame index on the write thread
	m_lastVideoFrameWriteIndex++;
}

void OpenCVVideoFrameBuffer::writeStereoVideoFrameSection(
	const unsigned char* video_buffer, 
	const cv::Rect& buffer_bounds, 
	bool bIsFlipped)
{
	EASY_FUNCTION();

	std::lock_guard<std::mutex> bufferLock(m_bufferMutex);
	const cv::Mat videoBufferMat(
		m_srcBufferHeight, m_srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(video_buffer));

	if (bIsFlipped)
	{
		cv::flip(videoBufferMat(buffer_bounds), *m_bgrBuffer, +1);
	}
	else
	{
		videoBufferMat(buffer_bounds).copyTo(*m_bgrBuffer);
	}

	// Atomically increment the frame index on the write thread
	m_lastVideoFrameWriteIndex++;
}

int64_t OpenCVVideoFrameBuffer::getLastVideoFrameWriteIndex() const
{
	return m_lastVideoFrameWriteIndex.load();
}

int64_t OpenCVVideoFrameBuffer::readVideoFrame(cv::Mat* outBGRBuffer, int64_t lastReadFrameIndex)
{
	EASY_FUNCTION();
	int64_t lastVideoFrameWriteIndex = getLastVideoFrameWriteIndex();

	if (lastVideoFrameWriteIndex != lastReadFrameIndex)
	{
		std::lock_guard<std::mutex> bufferLock(m_bufferMutex);

		m_bgrBuffer->copyTo(*outBGRBuffer);
	}

	return lastVideoFrameWriteIndex;
}