#pragma once

#include "AssetFwd.h"
#include "NodeFwd.h"

#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
#include "RendererFwd.h"
#include "NamedValueTable.h"
#include "GlFrameCompositorConfig.h"
#include "FrameCompositorConstants.h"
#include "ProfileConfigConstants.h"
#include "VideoDisplayConstants.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include <stdint.h>

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
		GlTexturePtr colorTexture;
		GlTexturePtr depthTexture;
		uint64_t frameIndex;
		bool bIsPendingRender;
	};

	static GlFrameCompositor* getInstance() { return m_instance; }

	GlFrameCompositor();
	virtual ~GlFrameCompositor();

	bool startup(class IGlWindow* ownerWindow);
	void shutdown();

	GlFrameCompositorConfigConstPtr getConfig() const { return m_config; }
	GlFrameCompositorConfigPtr getConfigMutable() { return m_config; }

	std::filesystem::path getCompositorPresetPath() const;
	void reloadAllCompositorPresets();
	std::vector<std::string> getPresetNames() const;
	const std::string& getCurrentPresetName() const;
	bool selectPreset(const std::string& configurationName, bool bForce= false);
	CompositorPresetConstPtr getCurrentPresetConfig() const { return m_currentPresetConfig; }
	CompositorPresetPtr getCurrentPresetConfigMutable() const { return m_currentPresetConfig; }
	bool addNewPreset();
	bool deleteCurrentPreset();
	bool setCurrentPresetName(const std::string& newPresetName);
	void saveCurrentPresetConfig();
	const std::filesystem::path& getCompositorGraphAssetPath() const;
	void setCompositorGraphAssetPath(const std::filesystem::path& assetRefPath);

	bool start();
	bool getIsRunning() const { return m_bIsRunning; }
	void stop();

	void update(float deltaSeconds);
	void render() const;

	bool getVideoSourceCameraPose(glm::mat4& outCameraMat) const;
	bool getVideoSourceViewProjection(glm::mat4& outCameraVP) const;
	inline VideoSourceViewPtr getVideoSource() const { return m_videoSourceView; }
	GlTexturePtr getVideoSourceTexture(eVideoTextureSource textureSource) const;

	inline const NamedValueTable<ClientSource*>& getClientSources() const { return m_clientSources; }
	GlTexturePtr getClientSourceTexture(int clientIndex, eClientTextureType clientTextureType) const;

	void setCompositorEvaluatorWindow(eCompositorEvaluatorWindow evalWindow);
	GlTexturePtr getEditorWritableFrameTexture() const;
	GlTextureConstPtr getCompositedFrameTexture() const;
	inline uint64_t getLastCompositedFrameIndex() const { return m_lastCompositedFrameIndex; }
	void setGenerateCompositedVideoFrame(bool bFlag) { m_bGenerateBGRVideoTexture = bFlag; }
	inline GlTexturePtr getBGRVideoFrameTexture() { return m_bgrVideoFrame; }
	void setGenerateBGRVideoTexture(bool bFlag) { m_bGenerateBGRVideoTexture= bFlag; }

	MulticastDelegate<void()> OnNewFrameComposited;

	static const struct GlVertexDefinition* getStencilModelVertexDefinition();

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
	void updateCompositeFrameNodeGraph();
	void updateCompositeFrameLayers();

	static const class GlProgramCode* getRGBFrameShaderCode();
	static const class GlProgramCode* getRGBtoBGRVideoFrameShaderCode();
	static const class GlProgramCode* getStencilShaderCode();

	// MikanServer Events
	void onClientRenderTargetAllocated(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetReleased(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor);
	void onClientRenderTargetUpdated(const std::string& clientId, uint64_t frameIndex);

	// Preset Config Events
	void onPresetConfigMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void onCompositorGraphAssetRefChanged(const std::string& assetRefPath);

private:
	//void rebuildAllLayerSettings(bool bForceConfigSave=false);
	void clearAllCompositorConfigurations();


	static GlFrameCompositor* m_instance;

	GlFrameCompositorConfigPtr m_config;
	CompositorPresetPtr m_currentPresetConfig;

	std::queue<MikanVideoSourceNewFrameEvent> m_frameEventQueue;

	VideoSourceViewPtr m_videoSourceView;
	VideoFrameDistortionView* m_videoDistortionView = nullptr;

	VRDeviceViewPtr m_cameraTrackingPuckView;

	class IGlWindow* m_ownerWindow= nullptr;

	unsigned int m_layerFramebuffer = 0;	
	unsigned int m_layerRBO = 0;
	unsigned int m_bgrFramebuffer = 0;
	unsigned int m_bgrRBO = 0;
	unsigned int m_videoQuadVAO = 0, m_videoQuadVBO = 0;
	unsigned int m_layerQuadVAO = 0, m_layerQuadVBO = 0;
	bool m_bGenerateBGRVideoTexture = false;
	GlProgramPtr m_rgbFrameShader = nullptr;
	GlProgramPtr m_rgbToBgrFrameShader = nullptr; // Keep

	eCompositorEvaluatorWindow m_evaluatorWindow = eCompositorEvaluatorWindow::mainWindow;
	GlTexturePtr m_editorFrameBufferTexture = nullptr;
	GlTexturePtr m_mainWindowFrameBufferTexture = nullptr;
	GlTexturePtr m_bgrVideoFrame = nullptr; // BGR, flipped video frame

	// Compositor Node Graph
	NodeGraphAssetReferencePtr m_nodeGraphAssetRef;
	CompositorNodeGraphPtr m_nodeGraph;

	// List of compositor presets from resources/config/compositor
	NamedValueTable<CompositorPresetPtr> m_compositorPresets;

	// Data sources used by the compositor layers
	NamedValueTable<ClientSource*> m_clientSources;

	bool m_bIsRunning= false;
	uint64_t m_lastReadVideoFrameIndex = 0;
	uint64_t m_droppedFrameCounter = 0;
	uint64_t m_lastCompositedFrameIndex = 0;
	uint64_t m_pendingCompositeFrameIndex = 0;
	float m_timeSinceLastFrameComposited= 0.f;
};