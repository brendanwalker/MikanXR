#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TransformComponent.h"
#include "MikanTypeFwd.h"
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

	MikanCameraID getCameraId() const { return m_CameraId; }

private:
	MikanCameraID m_CameraId;
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

	void extractCameraInfoForClientAPI(struct MikanCameraInfo& outCameraInfo) const;

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