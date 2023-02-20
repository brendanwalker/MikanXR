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

class GlState;
class GlTriangulatedMesh;
class VideoFrameDistortionView;

class GlMaterial;
typedef std::shared_ptr<GlMaterial> GlMaterialPtr;

class GlTexture;
typedef std::shared_ptr<GlTexture> GlTexturePtr;

class GlProgram;
typedef std::shared_ptr<GlProgram> GlProgramPtr;

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
		GlTexturePtr colorTexture;
		GlTexturePtr depthTexture;
		uint64_t frameIndex;
		bool bIsPendingRender;
	};

	struct Layer
	{
		int layerIndex;
		GlMaterialPtr layerMaterial;
		uint64_t frameIndex;
	};

	static GlFrameCompositor* getInstance() { return m_instance; }

	GlFrameCompositor();
	virtual ~GlFrameCompositor();

	bool startup();
	void shutdown();

	void reloadAllCompositorPresets();
	std::vector<std::string> getPresetNames() const;
	const std::string& getCurrentPresetName() const;
	bool applyLayerPreset(const std::string& configurationName);
	const CompositorPreset* getCurrentPresetConfig() const;
	CompositorPreset* getCurrentPresetConfigMutable() const;
	const CompositorLayerConfig* getCurrentPresetLayerConfig(int layerIndex) const;
	CompositorLayerConfig* getCurrentPresetLayerConfigMutable(int layerIndex);
	void saveCurrentPresetConfig();

	void reloadAllCompositorShaders();
	std::vector<std::string> getAllCompositorShaderNames() const;
	MulticastDelegate<void()> OnCompositorShadersReloaded;

	bool start();
	bool getIsRunning() const { return m_bIsRunning; }
	void stop();

	void update();
	void render() const;

	const class GlRenderModelResource* getStencilRenderModel(MikanStencilID stencilId) const;
	void flushStencilRenderModel(MikanStencilID stencilId);

	bool getVideoSourceCameraPose(glm::mat4& outCameraMat) const;
	inline VideoSourceViewPtr getVideoSource() const { return m_videoSourceView; }

	inline const NamedValueTable<float>& getFloatSources() const { return m_floatSources; }
	inline const NamedValueTable<glm::vec2>& getFloat2Sources() const { return m_float2Sources; }
	inline const NamedValueTable<glm::vec3>& getFloat3Sources() const { return m_float3Sources; }
	inline const NamedValueTable<glm::vec4>& getFloat4Sources() const { return m_float4Sources; }
	inline const NamedValueTable<glm::mat4>& getMat4Sources() const { return m_mat4Sources; }
	inline const NamedValueTable<GlTexturePtr>& getColorTextureSources() const { return m_colorTextureSources; }
	inline const NamedValueTable<ClientSource*>& getClientSources() const { return m_clientSources; }

	bool setLayerMaterialName(const int layerIndex, const std::string& materialName);
	void setIsLayerVerticalFlipped(const int layerIndex, bool bIsFlipped);
	void setLayerBlendMode(const int layerIndex, eCompositorBlendMode blendMode);

	void setFloatMapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void setFloat2Mapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void setFloat3Mapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void setFloat4Mapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void setMat4Mapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);
	void setColorTextureMapping(const int layerIndex, const std::string& uniformName, const std::string& dataSourceName);

	inline const std::vector<Layer>& getLayers() const { return m_layers; }
	inline const GlTexturePtr getCompositedFrameTexture() const { return m_compositedFrame; }
	void setGenerateCompositedVideoFrame(bool bFlag) { m_bGenerateBGRVideoTexture = bFlag; }
	inline GlTexturePtr getBGRVideoFrameTexture() { return m_bgrVideoFrame; }
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
\
	void updateCompositeFrame();
	void updateQuadStencils(const CompositorQuadStencilLayerConfig& stencilConfig, GlState* glState);
	void updateBoxStencils(const CompositorBoxStencilLayerConfig& stencilConfig, GlState* glState);
	void updateModelStencils(const CompositorModelStencilLayerConfig& stencilConfig, GlState* glState);

	static const class GlProgramCode* getRGBFrameShaderCode();
	static const class GlProgramCode* getRGBtoBGRVideoFrameShaderCode();
	static const class GlProgramCode* getStencilShaderCode();
	static const struct GlVertexDefinition* getStencilModelVertexDefinition();

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, uint64_t frameIndex);

private:
	void rebuildAllLayerSettings(bool bForceConfigSave=false);
	void clearAllCompositorConfigurations();

	bool applyLayerMaterialFloatValues(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool applyLayerMaterialFloat2Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool applyLayerMaterialFloat3Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool applyLayerMaterialFloat4Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool applyLayerMaterialMat4Values(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);
	bool applyLayerMaterialTextures(const CompositorLayerConfig& layerConfig, GlFrameCompositor::Layer& layer);

	static std::string makeClientRendererTextureName(int clientSourceIndex);
	static std::string makeClientColorKeyName(int clientSourceIndex);

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
	GlProgramPtr m_rgbFrameShader = nullptr;
	GlProgramPtr m_rgbToBgrFrameShader = nullptr; // Keep
	GlProgramPtr m_stencilShader = nullptr; // Keep
	GlTexturePtr m_compositedFrame = nullptr;
	GlTexturePtr m_bgrVideoFrame = nullptr; // BGR, flipped video frame

	// List of compositor presets from resources/config/compositor
	NamedValueTable<CompositorPreset*> m_compositorPresets;

	// Data sources used by the compositor layers
	NamedValueTable<float> m_floatSources;
	NamedValueTable<glm::vec2> m_float2Sources;
	NamedValueTable<glm::vec3> m_float3Sources;
	NamedValueTable<glm::vec4> m_float4Sources;
	NamedValueTable<glm::mat4> m_mat4Sources;
	NamedValueTable<GlTexturePtr> m_colorTextureSources;
	NamedValueTable<GlMaterialPtr> m_materialSources;
	NamedValueTable<ClientSource*> m_clientSources;

	bool m_bIsRunning= false;
	uint64_t m_lastReadVideoFrameIndex = 0;
	uint64_t m_droppedFrameCounter = 0;
	uint64_t m_lastCompositedFrameIndex = 0;
	uint64_t m_pendingCompositeFrameIndex = 0;
};

