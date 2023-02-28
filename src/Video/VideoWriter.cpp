#include "GlTexture.h"
#include "VideoWriter.h"
#include "Logger.h"

#include <assert.h>
#include <filesystem>
#include <chrono>
using namespace std::chrono_literals;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <inttypes.h>
}

#include "easy/profiler.h"

#define ENCODE_CIRCULAR_BUFFER_SIZE 2

VideoWriter::VideoWriter() : WorkerThread("VideoWriter")
{
}

VideoWriter::~VideoWriter()
{
	close();
}

bool VideoWriter::open(
	const std::filesystem::path& filename,
	float fps,
	int fwidth,
	int fheight,
	int bpp)
{
	if (m_bIsOpened)
		close();

	const std::string filenameString = filename.string();
	int error = avio_open(&m_avioContext, filenameString.c_str(), AVIO_FLAG_WRITE);
	if (error < 0)
	{
		char errorBuf[255];
		av_strerror(error, errorBuf, sizeof(errorBuf));
		MIKAN_LOG_ERROR("VideoWriter::get_ready_to_write") << "Could not open output file: " << errorBuf;
		return false;
	}

	m_filepath = filename;
	m_frameWidth = fwidth;
	m_frameHeight = fheight;
	m_fps = fps;
	m_bitsPerPixel = bpp;
	m_bgrFrameSizeBytes = (size_t)m_frameWidth * (size_t)m_frameHeight * (m_bitsPerPixel / 8);

	if (!allocateFrameBuffers())
		return false;

	if (!setupEncoder())
		return false;

	if (!addVideoStream())
		return false;

	if (!getIsReadyToWrite())
		return false;

	startThread();
	m_bIsOpened = true;

	return true;
}

void VideoWriter::close()
{
	if (!m_bIsOpened)
		return;

	stopThread();

	if (m_avFormatContext != nullptr)
	{
		av_write_trailer(m_avFormatContext);

		if (!(m_avFormatContext->flags & AVFMT_NOFILE))
			avio_closep(&m_avFormatContext->pb);

		avcodec_free_context(&m_avCodecContext);
		m_avFormatContext= nullptr;
	}

	freeFrameBuffers();
}

bool VideoWriter::getIsReadyToWrite(void)
{
	AVDictionary* param = 0;

	av_dict_set(&param, "preset", "fast", 0);
	av_dict_set(&param, "tune", "ull", 0);

	if (avcodec_open2(m_avCodecContext, m_codecEncoder, &param) < 0)
	{
		MIKAN_LOG_ERROR("VideoWriter::get_ready_to_write") << "Could not open codec context";
		return false;
	}

	if (avcodec_parameters_from_context(m_avStream->codecpar, m_avCodecContext) < 0)
	{
		MIKAN_LOG_ERROR("VideoWriter::get_ready_to_write") << "Could not copy params";
		return false;
	}

	if (avformat_write_header(m_avFormatContext, NULL) < 0)
	{
		MIKAN_LOG_ERROR("VideoWriter::get_ready_to_write") << "Could not copy params";
		return false;
	}
	
	return true;
}

bool VideoWriter::allocateFrameBuffers()
{
	m_avFrame = av_frame_alloc();
	if (!m_avFrame)
	{
		MIKAN_LOG_ERROR("VideoWriter::setup_frame") << "Could not allocate frame.";
		return false;
	}

	m_avFrame->format = AV_PIX_FMT_YUV420P; 
	m_avFrame->width = m_frameWidth;
	m_avFrame->height = m_frameHeight;
	if (av_frame_get_buffer(m_avFrame, 32) < 0)
	{
		MIKAN_LOG_ERROR("VideoWriter::setup_frame") << "System ran out of memory!";
		return false;
	}

	m_bgrToYUV420pConverter = sws_getContext(
		m_frameWidth,
		m_frameHeight,
		m_bitsPerPixel == 32 ? AV_PIX_FMT_BGRA : AV_PIX_FMT_BGR24,
		m_frameWidth,
		m_frameHeight,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL, NULL, NULL);
	if (!m_bgrToYUV420pConverter)
	{
		MIKAN_LOG_ERROR("VideoWriter::setup_frame") << "Cannot initialize converter";
		return false;
	}

	m_frameCirularBufferSize= ENCODE_CIRCULAR_BUFFER_SIZE;
	m_bgrFrameCirularBuffer = new uint8_t*[m_frameCirularBufferSize];
	for (int frameIndex = 0; frameIndex < m_frameCirularBufferSize; ++frameIndex)
	{
		m_bgrFrameCirularBuffer[frameIndex] = new uint8_t[m_bgrFrameSizeBytes];
		memset(m_bgrFrameCirularBuffer[frameIndex], 0, m_bgrFrameSizeBytes);
	}
	m_frameReadIndex = 0;
	m_frameWriteIndex = 0;

	return true;
}

void VideoWriter::freeFrameBuffers()
{
	if (m_bgrFrameCirularBuffer != nullptr)
	{
		for (int frameIndex = 0; frameIndex < m_frameCirularBufferSize; ++frameIndex)
		{
			delete[] m_bgrFrameCirularBuffer[frameIndex];
		}

		delete[] m_bgrFrameCirularBuffer;
		m_bgrFrameCirularBuffer= nullptr;
	}

	if (m_avFrame != nullptr)
		av_frame_free(&m_avFrame);

	if (m_bgrToYUV420pConverter != nullptr)
		sws_freeContext(m_bgrToYUV420pConverter);
}

bool VideoWriter::setupEncoder()
{
	//setup encoder and its context and properties
	m_codecEncoder = avcodec_find_encoder_by_name("h264_nvenc");
	if (m_codecEncoder == NULL)
	{
		MIKAN_LOG_ERROR("VideoWriter::setup_encoder") << "Could not load h264_nvenc encoder";
		return false;
	}
	else
	{
		MIKAN_LOG_INFO("VideoWriter::setup_encoder") << "Successfully found h264_nvenc encoder";
	}

	m_avCodecContext = avcodec_alloc_context3(m_codecEncoder);
	if (!m_avCodecContext)
	{
		MIKAN_LOG_ERROR("VideoWriter::setup_encoder") << "Could not allocate context for encoder";
		return false;
	}

	//allocate context for this encoder and set its properties
	m_avCodecContext->bit_rate = 8*1000*1000;
	m_avCodecContext->width = m_frameWidth;
	m_avCodecContext->height = m_frameHeight;


	return true;
}

bool VideoWriter::addVideoStream()
{
	const std::string filenameString = m_filepath.string();

	m_avFormatContext = avformat_alloc_context();
	if (!m_avFormatContext)
	{
		MIKAN_LOG_ERROR("VideoWriter::add_video_stream") << "Could not create output format context";
		return false;
	}

	m_avStream = avformat_new_stream(m_avFormatContext, NULL);//allocate new stream
	m_avFormatContext->pb = m_avioContext;
	m_avFormatContext->oformat = av_guess_format(NULL, filenameString.c_str(), NULL);//new AVOutputFormat;
	MIKAN_LOG_INFO("VideoWriter::add_video_stream") 
		<< "oformat name: " << m_avFormatContext->oformat->name 
		<< ", full: " << m_avFormatContext->oformat->long_name 
		<< ", codecid: " << m_avFormatContext->oformat->video_codec;

	AVRational fpsRational= { 100, (int)(100 * m_fps) };
	m_avStream->time_base = fpsRational;

	m_avCodecContext->time_base = m_avStream->time_base; //yay
	m_avCodecContext->gop_size = 12; //is this a typical value?
	m_avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	m_avFormatContext->oformat->video_codec = m_codecEncoder->id;

	return true;
}

bool VideoWriter::write(GlTexture* bgrTexture)
{
	EASY_FUNCTION();

	assert (bgrTexture != nullptr);

	if (!m_bIsOpened)
		return false;

	if (bgrTexture->getTextureWidth() != m_frameWidth ||
		bgrTexture->getTextureHeight() != m_frameHeight ||
		(size_t)bgrTexture->getBufferSize() != m_bgrFrameSizeBytes)
	{
		MIKAN_LOG_ERROR("VideoWriter::write") << "Texture format doesn't match video format";
		return false;
	}

	std::unique_lock<std::mutex> lock(m_frameMutex);

	// Make sure we aren't overflowing the circular frame buffer
	uint32_t nextWriteIndex = (m_frameWriteIndex + 1) % m_frameCirularBufferSize;
	if (nextWriteIndex == m_frameReadIndex)
	{
		MIKAN_LOG_WARNING("VideoWriter::write") << "bgrFrameCirularBuffer overflow. Dropping video write frame";
		return false;
	}

	// Fetch bgr texture data from the GPU and copy into the encoding circular buffer
	bgrTexture->copyTextureIntoBuffer(m_bgrFrameCirularBuffer[m_frameWriteIndex]);
	m_frameWriteIndex= nextWriteIndex;

	m_frameConsumerWait.notify_one();

	return true;
}

bool VideoWriter::doWork()
{
	EASY_FUNCTION();

	std::unique_lock<std::mutex> lock(m_frameMutex);
	m_frameConsumerWait.wait_for(lock, 100ms, [this] { return m_frameReadIndex != m_frameWriteIndex; });

	int bytesPerPixel = (m_bitsPerPixel / 8);
	int linesize[1] = { m_avCodecContext->width * bytesPerPixel };

	while (m_frameReadIndex != m_frameWriteIndex)
	{
		av_frame_make_writable(m_avFrame);

		uint8_t* bgrFrame = m_bgrFrameCirularBuffer[m_frameReadIndex];
		m_frameReadIndex = (m_frameReadIndex + 1) % m_frameCirularBufferSize;

		{
			EASY_BLOCK("sws_scale");

			if (sws_scale(
				m_bgrToYUV420pConverter,
				(const uint8_t* const*)&bgrFrame, linesize, 0, m_frameHeight,
				m_avFrame->data, m_avFrame->linesize) != m_frameHeight)
			{
				MIKAN_LOG_ERROR("VideoWriter::write") << "Error in scaling";
			}
		}

		m_avFrame->pts = m_presentationTimestamp++;

		{
			EASY_BLOCK("avcodec_send_frame");

			int ret = avcodec_send_frame(m_avCodecContext, m_avFrame);
			if (ret < 0)
			{
				MIKAN_LOG_ERROR("VideoWriter::write") << "Error submitting frame for encoding";
			}
		}

		{
			EASY_BLOCK("write_frame");

			AVPacket* avPacket = av_packet_alloc();

			int ret = avcodec_receive_packet(m_avCodecContext, avPacket);
			if (ret != 0)
			{
				char errorString[256];
				av_make_error_string(errorString, sizeof(errorString), ret);

				MIKAN_LOG_ERROR("VideoWriter::write") << errorString;
			}

			avPacket->flags |= AV_PKT_FLAG_KEY;
			avPacket->stream_index = m_avStream->index;
			av_packet_rescale_ts(avPacket, m_avCodecContext->time_base, m_avStream->time_base);

			ret = av_interleaved_write_frame(m_avFormatContext, avPacket);
			if (ret < 0)
			{
				char errorString[256];
				av_make_error_string(errorString, sizeof(errorString), ret);

				MIKAN_LOG_ERROR("VideoWriter::write") << "Error writing video frame: " << errorString;
			}

			av_packet_unref(avPacket);
			av_packet_free(&avPacket);
		}
	}

	return true;
}