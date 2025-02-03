#pragma once

#include "RendererFwd.h"

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>
#include <string>

namespace vr
{
	struct VRControllerState001_t;
	typedef VRControllerState001_t VRControllerState_t;
	struct RenderModel_ComponentState_t;
	struct RenderModel_ControllerMode_State_t;
};

class SteamVRRenderComponent
{
public:
	SteamVRRenderComponent() = default;
	SteamVRRenderComponent(
		class SteamVRDevice* ownerDevice,
		const std::string& _componentName,
		const std::string& _renderModelName,
		const bool _isRenderable);
	virtual ~SteamVRRenderComponent();

	bool initComponent();
	void updateComponent();
	void disposeComponent();

	void bindToScene(GlScenePtr scene);
	void removeFromBoundScene();

	inline class SteamVRDevice* getOwnerDevice() const { return m_ownerDevice; }
	inline const std::string& getComponentName() const { return m_componentName; }
	inline const std::string& getRenderModelName() const { return m_renderModelName; }
	inline bool getIsRenderable() const { return m_bIsRenderable; }
	inline GlStaticMeshInstancePtr getStaticMeshInstance() const { return m_glMeshInstance; }
	inline glm::mat4 getRenderPoseMatrix() const { return m_renderPoseMatrix; }
	inline glm::mat4 getComponentPoseMatrix() const { return m_componentPoseMatrix; }
	void setDiffuseColor(const glm::vec4& diffuseColor);

private:
	class SteamVRDevice* m_ownerDevice;
	std::string m_componentName;
	std::string m_renderModelName;
	bool m_bIsRenderable;
	GlStaticMeshInstancePtr m_glMeshInstance;
	vr::VRControllerState_t* m_controllerState;
	vr::RenderModel_ComponentState_t* m_componentState;
	vr::RenderModel_ControllerMode_State_t* m_componentModeState;
	glm::mat4 m_renderPoseMatrix;
	glm::mat4 m_componentPoseMatrix;
};
