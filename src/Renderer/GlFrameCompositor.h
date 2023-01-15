#pragma once

#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
#include "NamedValueTable.h"
#include "GlFrameCompositorConfig.h"
#include "FrameCompositorConstants.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include <stdint.h>

class GlMaterial;
class GlProgram;
class GlTexture;
class GlTriangulatedMesh;
class VideoFrameDistortionView;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class GlFrameCompositor
{
public:

	struct ClientSource
	{
		int clientSourceIndex;
		std::string clientId;
		MikanClientInfo clientInfo;
		MikanRenderTargetDescriptor desc;
		GlTexture* colorTexture;
		GlTexture* depthTexture;
		uint64_t frameIndex;
		bool bIsPendingRender;
	};

	struct Layer
	{
		int layerIndex;
		GlMaterial* layerMaterial;
		uint64_t frameIndex;
	};

	static GlFrameCompositor* getInstance() { return m_instance; }

	GlFrameCompositor();
	virtual ~GlFrameCompositor();

	bool startup();
	void shutdown();

	void reloadAllCompositorConfigurations();
	std::vector<std::string> getConfigurationNames() const;
	const std::string& getCurrentConfigurationName() const;
	bool setConfiguration(const std::string& configurationName);

	void reloadAllCompositorShaders();

	bool start();
	bool getIsRunning() const { return m_bIsRunning; }
	void stop();

	void update();
	void render() const;

	const class GlRenderModelResource* getStencilRenderModel(MikanStencilID stencilId) const;
	void flushStencilRenderModel(MikanStencilID stencilId);

	bool getVideoSourceCameraPose(glm::mat4& outCameraMat) const;
	inline VideoSourceViewPtr getVideoSource() const { return m_videoSourceView; }
	inline const NamedValueTable<ClientSource*>& getClientSources() const { return m_clientSources; }
	inline const std::vector<Layer>& getLayers() const { return m_layers; }
	inline const CompositorLayerConfig* getLayerConfig(int layerIndex) { 
		if (layerIndex >= 0 && layerIndex < (int)m_config.layers.size())
			return &m_config.layers[layerIndex]; 
		else
			return nullptr;
	}
	inline const GlTexture* getCompositedFrameTexture() const { return m_compositedFrame; }
	void setGenerateCompositedVideoFrame(bool bFlag) { m_bGenerateBGRVideoTexture = bFlag; }
	inline GlTexture* getBGRVideoFrameTexture() { return m_bgrVideoFrame; }
	void setGenerateBGRVideoTexture(bool bFlag) { m_bGenerateBGRVideoTexture= bFlag; }

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

	bool addClientSource(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	bool removeClientSource(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);

	void updateCompositeFrame();
	bool updateQuadStencils(const CompositorQuadStencilLayerConfig& stencilConfig);
	bool updateBoxStencils(const CompositorBoxStencilLayerConfig& stencilConfig);
	bool updateModelStencils(const CompositorModelStencilLayerConfig& stencilConfig);

	//static const class GlProgramCode* getRGBUndistortionFrameShaderCode();
	static const class GlProgramCode* getRGBFrameShaderCode();
	static const class GlProgramCode* getRGBtoBGRVideoFrameShaderCode();
	//static const class GlProgramCode* getRGBColorKeyLayerShaderCode();
	//static const class GlProgramCode* getRGBALayerShaderCode();
	static const class GlProgramCode* getStencilShaderCode();
	static const struct GlVertexDefinition* getStencilModelVertexDefinition();

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, uint64_t frameIndex);

private:
	void rebuildLayersFromConfig();
	void clearAllCompositorConfigurations();

	bool refreshLayerMaterialFloatValues(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool refreshLayerMaterialFloat2Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool refreshLayerMaterialFloat3Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool refreshLayerMaterialFloat4Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool refreshLayerMaterialMat4Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool refreshLayerMaterialTextures(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);

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
	//GlProgram* m_rgbUndistortionFrameShader= nullptr;
	GlProgram* m_rgbFrameShader = nullptr;
	GlProgram* m_rgbToBgrFrameShader = nullptr; // Keep
	//GlProgram* m_rgbColorKeyLayerShader = nullptr;
	//GlProgram* m_rgbaLayerShader = nullptr;
	GlProgram* m_stencilShader = nullptr; // Keep
	GlTexture* m_compositedFrame = nullptr;
	GlTexture* m_bgrVideoFrame = nullptr; // BGR, flipped video frame

	// List of compositor configurations from resources/config/compositor
	NamedValueTable<GlFrameCompositorConfig*> m_compositorConfigurations;

	// Data sources used by the compositor layers
	NamedValueTable<float> m_floatSources;
	NamedValueTable<glm::vec2> m_float2Sources;
	NamedValueTable<glm::vec3> m_float3Sources;
	NamedValueTable<glm::vec4> m_float4Sources;
	NamedValueTable<glm::mat4> m_mat4Sources;
	NamedValueTable<GlTexture*> m_colorTextureSources;
	NamedValueTable<GlMaterial*> m_materialSources;
	NamedValueTable<ClientSource*> m_clientSources;

	bool m_bIsRunning= false;
	uint64_t m_lastReadVideoFrameIndex = 0;
	uint64_t m_droppedFrameCounter = 0;
	uint64_t m_lastCompositedFrameIndex = 0;
	uint64_t m_pendingCompositeFrameIndex = 0;
};

