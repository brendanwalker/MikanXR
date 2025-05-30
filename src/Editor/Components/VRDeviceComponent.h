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

protected:
	VRDeviceViewPtr m_vrDeviceView;
	SelectionComponentWeakPtr m_selectionComponent;
};