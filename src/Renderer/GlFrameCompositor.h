#pragma once

#include <MikanClientTypes.h>
#include <MulticastDelegate.h>
#include "CommonConfig.h"
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include <stdint.h>

class GlProgram;
class GlTexture;
class GlTriangulatedMesh;
class VideoFrameDistortionView;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

enum class eCompositorLayerAlphaMode : int
{
	INVALID = -1,

	NoAlpha,
	ColorKey,
	AlphaChannel,
	MagicPortal,

	COUNT
};

struct CompositorLayerConfig
{
	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	std::string appName;
	eCompositorLayerAlphaMode alphaMode;
};

class GlFrameCompositorConfig : public CommonConfig
{
public:
	GlFrameCompositorConfig(const std::string& fnamebase = "FrameCompositorConfig")
	: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const CompositorLayerConfig& findOrAddDefaultLayerConfig(
		const MikanClientInfo& clientInfo,
		const MikanRenderTargetDescriptor& renderTargetDesc);
	CompositorLayerConfig* findLayerConfig(const MikanClientInfo& clientInfo);

	std::vector<CompositorLayerConfig> layers;
};

class GlFrameCompositor
{
public:

	struct Layer
	{
		std::string clientId;
		MikanClientInfo clientInfo;
		MikanRenderTargetDescriptor desc;
		eCompositorLayerAlphaMode alphaMode;
		GlTexture* colorTexture;
		GlTexture* depthTexture;
		uint64_t frameIndex;
		bool bIsPendingRender;
	};

	static GlFrameCompositor* getInstance() { return m_instance; }

	GlFrameCompositor();
	virtual ~GlFrameCompositor();

	bool startup();
	void shutdown();

	bool start();
	bool getIsRunning() const { return m_bIsRunning; }
	void stop();

	void update();
	void render() const;

	const class GlRenderModelResource* getStencilRenderModel(MikanStencilID stencilId) const;
	void flushStencilRenderModel(MikanStencilID stencilId);

	bool getVideoSourceCameraPose(glm::mat4& outCameraMat) const;
	inline VideoSourceViewPtr getVideoSource() const { return m_videoSourceView; }
	inline const std::vector<Layer>& getLayers() const { return m_layers; }
	inline const GlTexture* getCompositedFrameTexture() const { return m_compositedFrame; }
	void setGenerateCompositedVideoFrame(bool bFlag) { m_bGenerateBGRVideoTexture = bFlag; }
	inline GlTexture* getBGRVideoFrameTexture() { return m_bgrVideoFrame; }
	void setGenerateBGRVideoTexture(bool bFlag) { m_bGenerateBGRVideoTexture= bFlag; }
	void cycleNextLayerAlphaMode(int layerIndex);
	std::string getLayerAlphaModeString(int layerIndex) const;

	MulticastDelegate<void()> OnNewFrameComposited;

protected:

	bool openVideoSource();
	void closeVideoSource();

	bool bindCameraVRTracker();

	bool createLayerCompositingFrameBuffer(uint16_t width, uint16_t height);
	void freeLayerFrameBuffer();

	bool createBGRVideoFrameBuffer(uint16_t width, uint16_t height);
	void freeBGRVideoFrameBuffer();

	void createVertexBuffers();
	void freeVertexBuffers();

	void addLayer(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	void removeLayer(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);

	void updateCompositeFrame();
	bool updateStencils();
	static const class GlProgramCode* getRGBUndistortionFrameShaderCode();
	static const class GlProgramCode* getRGBFrameShaderCode();
	static const class GlProgramCode* getRGBtoBGRVideoFrameShaderCode();
	static const class GlProgramCode* getRGBColorKeyLayerShaderCode();
	static const class GlProgramCode* getRGBALayerShaderCode();
	static const class GlProgramCode* getStencilShaderCode();
	static const struct GlVertexDefinition* getStencilModelVertexDefinition();

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, uint64_t frameIndex);

private:
	static GlFrameCompositor* m_instance;

	GlFrameCompositorConfig m_config;

	std::queue<MikanVideoSourceNewFrameEvent> m_frameEventQueue;
	std::vector<Layer> m_layers;

	VideoSourceViewPtr m_videoSourceView;
	VideoFrameDistortionView* m_videoDistortionView = nullptr;

	VRDeviceViewPtr m_cameraTrackingPuckView;

	std::map<MikanStencilID, class GlRenderModelResource*> m_stencilMeshCache;

	unsigned int m_layerFramebuffer = 0;	
	unsigned int m_layerRBO = 0;
	unsigned int m_bgrFramebuffer = 0;
	unsigned int m_bgrRBO = 0;
	unsigned int m_videoQuadVAO = 0, m_videoQuadVBO = 0;
	unsigned int m_layerQuadVAO = 0, m_layerQuadVBO = 0;
	unsigned int m_stencilQuadVAO = 0, m_stencilQuadVBO = 0;
	unsigned int m_stencilBoxVAO = 0, m_stencilBoxVBO = 0;
	bool m_bGenerateBGRVideoTexture = false;
	GlProgram* m_rgbUndistortionFrameShader= nullptr;
	GlProgram* m_rgbFrameShader = nullptr;
	GlProgram* m_rgbToBgrFrameShader = nullptr;
	GlProgram* m_rgbColorKeyLayerShader = nullptr;
	GlProgram* m_rgbaLayerShader = nullptr;
	GlProgram* m_stencilShader = nullptr;
	GlTexture* m_compositedFrame = nullptr;
	GlTexture* m_bgrVideoFrame = nullptr; // BGR, flipped video frame

	bool m_bIsRunning= false;
	uint64_t m_lastReadVideoFrameIndex = 0;
	uint64_t m_droppedFrameCounter = 0;
	uint64_t m_lastCompositedFrameIndex = 0;
	uint64_t m_pendingCompositeFrameIndex = 0;
};

