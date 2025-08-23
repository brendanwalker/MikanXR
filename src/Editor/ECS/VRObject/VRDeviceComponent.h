#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "DeviceViewFwd.h"
#include "TransformComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <map>
#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

// -- constants -----
enum class eVRDevicePoseSpace : int
{
	INVALID,

	VRTrackingSystem,
	MikanScene,

	COUNT,
};

// -- VRDeviceDefinition -----
class VRDeviceDefinition : public TransformComponentDefinition
{
public:
	VRDeviceDefinition() = default;
	VRDeviceDefinition(
		MikanVRDeviceID vrDeviceId,
		const std::string& vrDevicePath,
		const MikanTransform& xform);

	inline MikanVRDeviceID getVRDeviceId() const { return m_vrDeviceId; }
	inline const std::string getVRDevicePath() const { return m_vrDevicePath; }

private:
	MikanVRDeviceID m_vrDeviceId= -1;
	std::string m_vrDevicePath;
};

// -- VRDeviceComponent -----
class VRDeviceComponent : public TransformComponent
{
public:
	VRDeviceComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline VRDeviceDefinitionPtr getVRDeviceDefinition() const
	{
		return std::static_pointer_cast<VRDeviceDefinition>(m_definition);
	}

	void setVRDeviceInterface(class IVRDevice* vrDeviceInterface);
	class IVRDevice* getVRDeviceInterface() const { return m_vrDeviceInterface; }

	bool getDevicePose(const int vrFrameDelay, struct VRDevicePose& outPose) const;

	void disposeSockets();
	void rebuildSockets();
	bool getSocketRelativePoseByName(const std::string& socketName, glm::mat4& outPose) const;
	bool getDefaultSocketRelativePose(glm::mat4& outPose) const;
	VRDevicePoseViewPtr makePoseView(
		eVRDevicePoseSpace space,
		const std::string& socketName= "") const;

	void disposeMeshComponents();
	void rebuildMeshComponents();

	void refreshDevicePose();

protected:
	void updateWireframeMeshColor();

	// Selection Events
	void onInteractionRayOverlapEnter(const struct ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const struct ColliderRaycastHitResult& hitResult);
	void onInteractionSelected();
	void onInteractionUnselected();

protected:
	struct VRDeviceMeshInfo
	{
		StaticMeshComponentPtr wireStaticMeshComponent;
		StaticMeshComponentPtr triStaticMeshComponent;
		MeshColliderComponentPtr colliderComponent;
	};

	class IVRDevice* m_vrDeviceInterface= nullptr;
	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::map<std::string, TransformComponentPtr> m_socketMap;
	std::map<std::string, VRDeviceMeshInfo> m_meshComponentMap;
	bool m_bIsHovered = false;
	bool m_bIsSelected = false;
};

// -- VRDevicePoseView -----
class VRDevicePoseView
{
public:
	VRDevicePoseView();
	VRDevicePoseView(
		const VRDeviceComponent* deviceComponent,
		eVRDevicePoseSpace space,
		const std::string& socketName = "");

	static VRDevicePoseViewPtr makePoseView(
		const VRDeviceComponent* deviceComponent,
		eVRDevicePoseSpace space,
		const std::string& socketName = "");
	static VRDevicePoseViewPtr makeInvalidPoseView();

	inline eVRDevicePoseSpace getPoseSpace() const { return m_poseSpace; }

	const VRDeviceComponent* getDeviceComponent() const;
	bool getPose(glm::mat4& outPoseInSpace) const;
	bool getPose(glm::dmat4& outPoseInSpace) const;

private:
	VRDeviceComponentConstWeakPtr m_deviceComponent;
	eVRDevicePoseSpace m_poseSpace;
	std::string m_socketName;
};