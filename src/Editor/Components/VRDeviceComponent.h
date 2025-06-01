#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "VRDeviceView.h"
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

	void assignToStage(MikanStageID newStageId);
	StageComponentPtr getAssignedStage() const;
	MikanStageID getAssignedStageId() const;

	void disposeSockets();
	void rebuildSockets();

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