#pragma once

#include "VideoDisplayConstants.h"
#include "OpenCVFwd.h"
#include "RendererFwd.h"
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

	inline eVideoDisplayMode getVideoDisplayMode() const { return m_videoDisplayMode; }
	inline void setVideoDisplayMode(eVideoDisplayMode newMode) { m_videoDisplayMode= newMode; }

	inline bool isColorUndistortDisabled() const { return m_bColorUndistortDisabled; }
	inline void setColorUndistortDisabled(bool bDisabled) { m_bColorUndistortDisabled= bDisabled; }

	inline bool isGrayscaleUndistortDisabled() const { return m_bGrayscaleUndistortDisabled; }
	inline void setGrayscaleUndistortDisabled(bool bDisabled) { m_bGrayscaleUndistortDisabled = bDisabled; }

	inline unsigned int getMaxFrameQueueSize() const { return m_bgrSourceBufferCount; }
	inline uint64_t getLastVideoFrameReadIndex() const { return m_lastVideoFrameReadIndex; }

	inline cv::Mat* getGrayscaleSourceBuffer() const { return m_gsSourceBuffer; }
	inline cv::Mat* getGrayscaleUndistortBuffer() const { return m_gsUndistortBuffer; }
	inline cv::Mat* getBGRUndistortBuffer() const { return m_bgrUndistortBuffer; }
	inline cv::Mat* getBGRGsDisplayBuffer() const { return m_bgrGsDisplayBuffer; }
	inline GlTexturePtr getDistortionTexture() const { return m_distortionTextureMap; }
	inline GlTexturePtr getVideoTexture() const { return m_videoTexture; }

	bool hasNewVideoFrame() const;
	uint64_t readNextVideoFrame();
	bool processVideoFrame(uint64_t desiredFrameIndex);
	bool readAndProcessVideoFrame();
	void rebuildDistortionMap(const struct MikanMonoIntrinsics* instrinsics);

	void renderSelectedVideoBuffers();

protected:
	void createLayerQuadMesh();
	void computeUndistortion(cv::Mat* bgrSourceBuffer);

	static void copyOpenCVMatIntoGLTexture(const cv::Mat& mat, GlTexturePtr texture);

protected:
	IGlWindow* m_ownerWindow= nullptr;

	eVideoDisplayMode m_videoDisplayMode;
	VideoSourceViewPtr m_videoSourceView;
	int m_frameWidth;
	int m_frameHeight;
	
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
	cv::Mat* m_gsUndistortBuffer; // 8-BPP undistorted buffer
	cv::Mat* m_bgrGsDisplayBuffer; // 24-BPP(BGR color format) debug display buffer

	// Camera Intrinsics / Distortion parameters
	struct OpenCVMonoCameraIntrinsics* m_intrinsics;

	// Distortion preview
	cv::Mat* m_distortionMapX;
	cv::Mat* m_distortionMapY;
	GlTexturePtr m_distortionTextureMap= nullptr;

	// Texture used for display
	GlTexturePtr m_videoTexture = nullptr;

	// Quad used for fullscreen rendering
	GlTriangulatedMeshPtr m_fullscreenQuad;

	// Runtime flags
	bool m_bColorUndistortDisabled= false;
	bool m_bGrayscaleUndistortDisabled = false;
};
