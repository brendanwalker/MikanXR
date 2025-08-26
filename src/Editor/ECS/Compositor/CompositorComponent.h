#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "FrameCompositorConstants.h"
#include "MikanCameraEvents.h"
#include "MikanComponent.h"
#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "MkRendererFwd.h"
#include "NodeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"
#include "VideoDisplayConstants.h"

#include <memory>
#include <queue>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class VideoFrameDistortionView;
using VideoFrameDistortionViewPtr = std::shared_ptr<VideoFrameDistortionView>;

class CompositorDefinition : public MikanComponentDefinition
{
public:
	CompositorDefinition();
	CompositorDefinition(
		MikanCompositorID compositorId,
		MikanSceneID ownerSceneId,
		const std::string& compositorName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanCompositorID getCompositorId() const { return m_compositorId; }

	static const std::string k_cameraPropertyId;
	inline MikanCameraID getCameraId() const { return m_cameraId; }
	void setCameraId(MikanCameraID cameraId);

	static const std::string k_ownerScenePropertyId;
	inline MikanSceneID getOwnerSceneId() const { return m_ownerSceneId; }
	void setOwnerSceneId(MikanSceneID sceneId);

	static const std::string k_compositorGraphPathPropertyId;
	bool hasCompositorGraphPath() const;
	std::filesystem::path getCompositorGraphPath() const;
	void setCompositorGraphPath(const std::filesystem::path& graphPath);

private:
	MikanCompositorID m_compositorId;
	MikanSceneID m_ownerSceneId = INVALID_MIKAN_ID;
	MikanCameraID m_cameraId = INVALID_MIKAN_ID;
	AssetReferenceConfigPtr m_nodeGraphAssetRef;
};

class CompositorComponent : public MikanComponent
{
public:
	CompositorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;
	virtual void update(float deltaSeconds) override;
	void render() const;

	bool start();
	bool getIsRunning() const { return m_bIsRunning; }
	void stop();

	SceneComponentPtr getOwnerSceneComponent() const;
	VideoSourceComponentPtr getVideoSourceComponent() const;

	CameraComponentPtr getCameraComponent() const;
	void setCameraComponent(CameraComponentPtr cameraComponent);

	std::filesystem::path getCompositorGraphAssetPath() const;
	void setCompositorGraphAssetPath(const std::filesystem::path& assetRefPath);

	inline CompositorDefinitionPtr getCompositorDefinition() const
	{
		return std::static_pointer_cast<CompositorDefinition>(m_definition);
	}
	inline MikanCompositorID getCompositorId() const 
	{ 
		return getCompositorDefinition()->getCompositorId(); 
	}

	IMkTexturePtr getVideoSourceTexture(eVideoTextureSource textureSource) const;
	IMkTexturePtr getVideoPreviewTexture(eVideoTextureSource textureSource) const;
	void setCompositorEvaluatorWindow(eCompositorEvaluatorWindow evalWindow);
	IMkTexturePtr getEditorWritableFrameTexture() const;
	IMkTextureConstPtr getCompositedFrameTexture() const;
	inline int64_t getLastCompositedFrameIndex() const { return m_lastCompositedFrameIndex; }

	MulticastDelegate<void()> OnNewFrameComposited;


	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

protected:
	void handleCompositorNodeGraphChanged(const std::filesystem::path& newAssetRefPath);
	void handleCameraChange(CameraComponentPtr oldCameraComponent, CameraComponentPtr newCameraComponent);
	void unbindVideoSourceEvents(VideoSourceComponentPtr videoSource);
	void bindVideoSourceEvents(VideoSourceComponentPtr videoSource);
	void disposeVideoBuffers();
	void allocateVideoBuffers(VideoSourceComponentPtr videoSource);
	void createCompositingTextures(int width, int height);
	void disposeCompositingTextures();
	void onVideoFrameSizeChanged(VideoSourceComponentPtr videoSource);
	void updateCompositeFrame();
	void updateCompositeFrameNodeGraph();

private:
	// Compositor Rendering
	IMkTriangulatedMeshPtr m_layerQuadMesh;

	// Pending queue of camera poses awaiting client render
	std::queue<MikanCameraNewFrameEvent> m_frameEventQueue;

	// Shared Texture for editor window rendering
	eCompositorEvaluatorWindow m_evaluatorWindow = eCompositorEvaluatorWindow::mainWindow;
	IMkTexturePtr m_editorFrameBufferTexture = nullptr;

	// Compositor Node Graph
	NodeGraphAssetReferencePtr m_nodeGraphAssetRef;
	CompositorNodeGraphPtr m_nodeGraph;

	// Undistorted Video Frame Buffer
	VideoFrameDistortionViewPtr m_videoDistortionView;

	bool m_bIsRunning = false;
	int64_t m_lastReadVideoFrameIndex = 0;
	int64_t m_droppedFrameCounter = 0;
	int64_t m_lastCompositedFrameIndex = 0;
	int64_t m_pendingCompositeFrameIndex = 0;
	float m_timeSinceLastFrameComposited= 0.f;
};
