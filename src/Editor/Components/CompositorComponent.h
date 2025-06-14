#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanCoreTypes.h"
#include "MikanTypeFwd.h"
#include "MkRendererFwd.h"
#include "NodeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

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

	bool getIsRunning() const;
	SceneComponentPtr getOwnerSceneComponent() const;
	CameraComponentPtr getCameraComponent() const;

	inline CompositorDefinitionPtr getCompositorDefinition() const
	{
		return std::static_pointer_cast<CompositorDefinition>(m_definition);
	}

	IMkTextureConstPtr getCompositedFrameTexture() const;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

protected:
	// Compositor Rendering
	IMkTriangulatedMeshPtr m_layerQuadMesh;

	// Compositor Node Graph
	NodeGraphAssetReferencePtr m_nodeGraphAssetRef;
	CompositorNodeGraphPtr m_nodeGraph;

	int64_t m_pendingCompositeFrameIndex = 0;
	float m_timeSinceLastFrameComposited= 0.f;
};
