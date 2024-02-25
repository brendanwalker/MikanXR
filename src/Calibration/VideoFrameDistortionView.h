#pragma once

#include "VideoDisplayConstants.h"
#include "MikanClientTypes.h"
#include "OpenCVFwd.h"
#include <memory>

class GlTexture;
typedef std::shared_ptr<GlTexture> GlTexturePtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VideoFrameDistortionView
{
public:
	VideoFrameDistortionView(
		VideoSourceViewPtr view, 
		unsigned int bufferBitmask, 
		unsigned int frameQueueSize=1);
	virtual ~VideoFrameDistortionView();

	inline VideoSourceViewPtr getVideoSourceView() const { return m_videoSourceView; }

	inline int getFrameWidth() const { return m_frameWidth; }
	inline int getFrameHeight() const { return m_frameHeight; }
	inline float getSmallToSourceScale() const { return m_gsSmallToSourceScale; }

	inline eVideoDisplayMode getVideoDisplayMode() const { return m_videoDisplayMode; }
	inline void setVideoDisplayMode(eVideoDisplayMode newMode) { m_videoDisplayMode= newMode; }
	inline void setColorUndistortDisabled(bool bDisabled) { m_bColorUndistortDisabled= bDisabled; }
	inline void setGrayscaleUndistortDisabled(bool bDisabled) { m_bGrayscaleUndistortDisabled = bDisabled; }

	inline unsigned int getMaxFrameQueueSize() const { return m_bgrSourceBufferCount; }
	inline uint64_t getLastVideoFrameReadIndex() const { return m_lastVideoFrameReadIndex; }

	inline cv::Mat* getBGRUndistortBuffer() const { return m_bgrUndistortBuffer; }
	inline cv::Mat* getBGRGrayscaleUndistortBuffer() const { return m_bgrGsUndistortBuffer; }
	inline cv::Mat* getGrayscaleSourceBuffer() const { return m_gsSourceBuffer; }
	inline cv::Mat* getGrayscaleSmallBuffer() const { return m_gsSmallBuffer; }
	inline GlTexturePtr getDistortionTexture() const { return m_distortionTextureMap; }
	inline GlTexturePtr getVideoTexture() const { return m_videoTexture; }

	bool hasNewVideoFrame() const;
	uint64_t readNextVideoFrame();
	bool processVideoFrame(uint64_t desiredFrameIndex);
	bool readAndProcessVideoFrame();
	void rebuildDistortionMap(const MikanMonoIntrinsics* instrinsics);

	void renderSelectedVideoBuffers();

protected:
	void computeUndistortion(cv::Mat* bgrSourceBuffer);

	static void copyOpenCVMatIntoGLTexture(const cv::Mat& mat, GlTexturePtr texture);

protected:	
	eVideoDisplayMode m_videoDisplayMode;
	VideoSourceViewPtr m_videoSourceView;
	int m_frameWidth;
	int m_frameHeight;
	float m_gsSmallToSourceScale;

	// Circular BGR source buffer
	struct SourceBufferEntry
	{
		cv::Mat* bgrSourceBuffer;
		uint64_t frameIndex;
	};
	SourceBufferEntry* m_bgrSourceBuffers;
	unsigned int m_bgrSourceBufferCount;
	unsigned int m_bgrSourceBufferWriteIndex;
	uint64_t m_lastVideoFrameReadIndex;

	// Video frame buffers (24-BPP, BGR color format)
	cv::Mat* m_bgrSourceBuffer_OGL; // 24-BPP(BGR color format) source buffer on GPU
	cv::Mat* m_bgrUndistortBuffer;

	// Grayscale video frame buffers
	cv::Mat* m_gsSourceBuffer; // 8-BPP source buffer
	cv::Mat* m_gsSmallBuffer; // Smaller version of the 8-BPP source buffer
	cv::Mat* m_gsUndistortBuffer; // 8-BPP undistorted buffer
	cv::Mat* m_bgrGsUndistortBuffer; // 24-BPP(BGR color format) undistorted buffer

	// Camera Intrinsics / Distortion parameters
	struct OpenCVMonoCameraIntrinsics* m_intrinsics;

	// Distortion preview
	cv::Mat* m_distortionMapX;
	cv::Mat* m_distortionMapY;
	GlTexturePtr m_distortionTextureMap= nullptr;

	// Texture used for display
	GlTexturePtr m_videoTexture = nullptr;

	// Runtime flags
	bool m_bColorUndistortDisabled= false;
	bool m_bGrayscaleUndistortDisabled = false;
};
