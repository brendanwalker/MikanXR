#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TransformComponent.h"
#include "MikanTypeFwd.h"
#include "MikanCameraTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class CameraDefinition : public TransformComponentDefinition
{
public:
	CameraDefinition();
	CameraDefinition(
		MikanCameraID cameraId, 
		const std::string& cameraName,
		const struct MikanTransform& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanCameraID getCameraId() const { return m_cameraId; }
	MikanCameraInfo getCameraInfo() const;

	static const std::string k_attachedVRDevicePropertyId;
	MikanVRDeviceID getAttachedVRDeviceId() const { return m_attachedVRDeviceId; }
	void setAttachedVRDeviceId(MikanVRDeviceID deviceId);

	static const std::string k_orientationOffsetPropertyId;
	MikanQuatd getOrientationOffset() const { return m_orientationOffset; }
	void setOrientationOffset(MikanQuatd rotation);

	static const std::string k_positionOffsetPropertyId;
	MikanVector3d getPositionOffset() const { return m_positionOffset; }
	void setPositionOffset(MikanVector3d translation);

private:
	MikanCameraID m_cameraId;
	MikanVRDeviceID m_attachedVRDeviceId;
	MikanQuatd m_orientationOffset;
	MikanVector3d m_positionOffset;
};

class CameraComponent : public TransformComponent
{
public:
	CameraComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline CameraDefinitionPtr getCameraDefinition() const
	{
		return std::static_pointer_cast<CameraDefinition>(m_definition);
	}

	// -- IFunctionInterface ----
	static const std::string k_editCameraFunctionId;
	static const std::string k_deleteCameraFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void editCamera();
	void deleteCamera();

protected:
	SelectionComponentWeakPtr m_selectionComponent;
};