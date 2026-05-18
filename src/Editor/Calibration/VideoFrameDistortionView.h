#pragma once

#include "IEditorWindow.h"
#include "VideoDisplayConstants.h"
#include "OpenCVFwd.h"
#include "MikanRendererFwd.h"

#include <memory>
#include <chrono>

class VideoSourceComponent;
typedef std::shared_ptr<VideoSourceComponent> VideoSourceComponentPtr;

class VideoFrameDistortionView
{
public:
	VideoFrameDistortionView(
		VideoSourceComponentPtr view, 
		unsigned int bufferBitmask, 
		unsigned int frameQueueSize=1);
	virtual ~VideoFrameDistortionView();

	inline VideoSourceComponentPtr getVideoSourceComponent() const { return m_videoSourceComponent; }
	inline IMkGraphicsContext* getGraphicsContext() const;

	inline int getFrameWidth() const { return m_frameWidth; }
	inline int getFrameHeight() const { return m_frameHeight; }
	inline float getFPS() const { return m_fps; }

	inline eVideoDisplayMode getVideoDisplayMode() const { return m_videoDisplayMode; }
	inline void setVideoDisplayMode(eVideoDisplayMode newMode) { m_videoDisplayMode= newMode; }

	inline bool isColorUndistortDisabled() const { return m_bColorUndistortDisabled; }
	inline void setColorUndistortDisabled(bool bDisabled) { m_bColorUndistortDisabled= bDisabled; }

	inline bool isGrayscaleUndistortDisabled() const { return m_bGrayscaleUndistortDisabled; }
	inline void setGrayscaleUndistortDisabled(bool bDisabled) { m_bGrayscaleUndistortDisabled = bDisabled; }

	inline unsigned int getMaxFrameQueueSize() const { return m_videoFrameQueueSize; }
	inline int64_t getLastVideoFrameReadIndex() const { return m_lastVideoFrameReadIndex; }
	inline cv::Mat* getGrayscaleSourceBuffer() const { return m_gsSourceBuffer; }
	inline cv::Mat* getGrayscaleUndistortBuffer() const { return m_gsUndistortBuffer; }
	inline cv::Mat* getBGRUndistortBuffer() const { return m_bgrUndistortBuffer; }
	inline cv::Mat* getBGRGsDisplayBuffer() const { return m_bgrGsDisplayBuffer; }
	inline IMkTexturePtr getDistortionTexture() const { return m_distortionTextureMap; }

	bool hasNewVideoFrame() const;
	int64_t readAndProcessVideoFrame();
	IMkTexturePtr getVideoTexture(int64_t desiredFrameIndex = -1) const;
	void applyMonoCameraIntrinsics(const struct MikanMonoIntrinsics* instrinsics);

	void renderSelectedVideoBuffers();

protected:
	int64_t readNextVideoFrame();
	void processVideoFrame(int64_t newFrameIndex);
	void ensureFrameBufferSize(int width, int height);
	void rebuildDistortionMap();
	void computeUndistortion(cv::Mat* bgrSourceBuffer);

	static void copyOpenCVMatIntoGLTexture(const cv::Mat& mat, IMkTexturePtr texture);

protected:
	eVideoDisplayMode m_videoDisplayMode;
	VideoSourceComponentPtr m_videoSourceComponent;
	bool m_bVideoIsStreaming;
	unsigned int m_bufferBitmask;
	int m_frameWidth;
	int m_frameHeight;
	float m_fps;
	
	// BGR source buffer
	cv::Mat* m_bgrSourceBuffer;
	int64_t m_lastVideoFrameReadIndex;
	std::chrono::steady_clock::time_point m_lastFrameTimestamp;

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

	// Circular RGB video frame queue
	struct VideoFrameQueueEntry
	{
		IMkTexturePtr videoTexture;
		int64_t frameIndex;
	};
	VideoFrameQueueEntry* m_videoFrameQueue = nullptr;
	unsigned int m_videoFrameQueueSize= 0;
	int m_videoFrameQueueLastWriteIndex= -1;
	int m_videoFrameQueuePendingWriteIndex = 0;

	// Quad used for fullscreen video rendering
	IMkTriangulatedMeshPtr m_fullscreenRGBVideoQuad;

	// Quad used for fullscreen rendering when no video is available
	IMkTriangulatedMeshPtr m_fullscreenRGBNoVideoQuad;

	// Runtime flags
	bool m_bColorUndistortDisabled= false;
	bool m_bGrayscaleUndistortDisabled = false;
};
