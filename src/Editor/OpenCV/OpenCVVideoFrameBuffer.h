#include "VideoSourceInterface.h"
#include "OpenCVFwd.h"

#include <atomic>
#include <mutex>

class OpenCVVideoFrameBuffer
{
public:
	OpenCVVideoFrameBuffer(
		int bufferPixelWidth, int bufferPixelHeight,
		int framePixelWidth, int framePixelHeight,
		VideoFrameSection _section);
	virtual ~OpenCVVideoFrameBuffer();

	void writeVideoFrame(const unsigned char* video_buffer, bool bIsFlipped);
	void writeStereoVideoFrameSection(const unsigned char* video_buffer, const cv::Rect& buffer_bounds, bool bIsFlipped);
	int64_t getLastVideoFrameWriteIndex() const;
	int64_t readVideoFrame(cv::Mat* outBGRBuffer, int64_t lastReadFrameIndex);

private:
	VideoFrameSection m_section;

	int m_srcBufferWidth;
	int m_srcBufferHeight;
	int m_frameWidth;
	int m_frameHeight;

	std::mutex m_bufferMutex;
	cv::Mat* m_bgrBuffer; // source video frame
	std::atomic_int64_t m_lastVideoFrameWriteIndex;
};