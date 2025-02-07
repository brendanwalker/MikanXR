#pragma once

#include "VideoDisplayConstants.h"
#include "OpenCVFwd.h"
#include "MikanRendererFwd.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VideoFrameDistortionView
{
public:
	VideoFrameDistortionView(
		class IGlWindow* ownerWindow,
		VideoSourceViewPtr view, 
		unsigned int bufferBitmask, 
		unsigned int frameQueueSize=1);
	virtual ~VideoFrameDistortionView();

	inline VideoSourceViewPtr getVideoSourceView() const { return m_videoSourceView; }

	inline int getFrameWidth() const { return m_frameWidth; }
	inline int getFrameHeight() const { return m_frameHeight; }
	inline float getFPS() const { return m_fps; }

	inline eVideoDisplayMode getVideoDisplayMode() const { return m_videoDisplayMode; }
	inline void setVideoDisplayMode(eVideoDisplayMode newMode) { m_videoDisplayMode= newMode; }

	inline bool isColorUndistortDisabled() const { return m_bColorUndistortDisabled; }
	inline void setColorUndistortDisabled(bool bDisabled) { m_bColorUndistortDisabled= bDisabled; }

	inline bool isGrayscaleUndistortDisabled() const { return m_bGrayscaleUndistortDisabled; }
	inline void setGrayscaleUndistortDisabled(bool bDisabled) { m_bGrayscaleUndistortDisabled = bDisabled; }

	inline unsigned int getMaxFrameQueueSize() const { return m_bgrSourceBufferCount; }
	inline int64_t getLastVideoFrameReadIndex() const { return m_lastVideoFrameReadIndex; }

	inline cv::Mat* getGrayscaleSourceBuffer() const { return m_gsSourceBuffer; }
	inline cv::Mat* getGrayscaleUndistortBuffer() const { return m_gsUndistortBuffer; }
	inline cv::Mat* getBGRUndistortBuffer() const { return m_bgrUndistortBuffer; }
	inline cv::Mat* getBGRGsDisplayBuffer() const { return m_bgrGsDisplayBuffer; }
	inline IMkTexturePtr getDistortionTexture() const { return m_distortionTextureMap; }
	inline IMkTexturePtr getVideoTexture() const { return m_videoTexture; }

	bool hasNewVideoFrame() const;
	int64_t readNextVideoFrame();
	bool processVideoFrame(int64_t desiredFrameIndex);
	bool readAndProcessVideoFrame();
	void applyMonoCameraIntrinsics(const struct MikanMonoIntrinsics* instrinsics);

	void renderSelectedVideoBuffers();

protected:
	void ensureFrameBufferSize(int width, int height);
	void rebuildDistortionMap();
	void computeUndistortion(cv::Mat* bgrSourceBuffer);

	static void copyOpenCVMatIntoGLTexture(const cv::Mat& mat, IMkTexturePtr texture);

protected:
	IGlWindow* m_ownerWindow= nullptr;

	eVideoDisplayMode m_videoDisplayMode;
	VideoSourceViewPtr m_videoSourceView;
	unsigned int m_bufferBitmask;
	int m_frameWidth;
	int m_frameHeight;
	float m_fps;
	
	// Circular BGR source buffer
	struct SourceBufferEntry
	{
		cv::Mat* bgrSourceBuffer;
		int64_t frameIndex;
	};
	SourceBufferEntry* m_bgrSourceBuffers;
	unsigned int m_bgrSourceBufferCount;
	unsigned int m_bgrSourceBufferWriteIndex;
	int64_t m_lastVideoFrameReadIndex;
	uint32_t m_lastFrameTimestamp;

	// Video frame buffers (24-BPP, BGR color format)
	cv::Mat* m_bgrSourceBuffer_OGL; // 24-BPP(BGR color format) source buffer on GPU
	cv::Mat* m_bgrUndistortBuffer;

	// Grayscale video frame buffers
	cv::Mat* m_gsSourceBuffer; // 8-BPP source buffer
	cv::Mat* m_gsUndistortBuffer; // 8-BPP undistorted buffer
	cv::Mat* m_bgrGsDisplayBuffer; // 24-BPP(BGR color format) debug display buffer

	// Camera Intrinsics / Distortion parameters
	struct OpenCVMonoCameraIntrinsics* m_intrinsics;

	// Distortion preview
	cv::Mat* m_distortionMapX;
	cv::Mat* m_distortionMapY;
	IMkTexturePtr m_distortionTextureMap= nullptr;

	// Texture used for display
	IMkTexturePtr m_videoTexture = nullptr;

	// Quad used for fullscreen rendering
	IMkTriangulatedMeshPtr m_fullscreenQuad;

	// Runtime flags
	bool m_bColorUndistortDisabled= false;
	bool m_bGrayscaleUndistortDisabled = false;
};
