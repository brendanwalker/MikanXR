#include "VRDeviceComponent.h"
#include "VRObjectSystem.h"
#include "App.h"
#include "Colors.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "MainWindow.h"
#include "MathGLM.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MikanVRDeviceTypes.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

// -- VRDeviceConfig -----
VRDeviceDefinition::VRDeviceDefinition(
	MikanVRDeviceID vrDeviceId,
	const std::string& vrDevicePath,
	const MikanTransform& xform)
	: TransformComponentDefinition(vrDevicePath, xform)
	, m_vrDeviceId(vrDeviceId)
	, m_vrDevicePath(vrDevicePath)
{}

// -- VRDeviceComponent -----
VRDeviceComponent::VRDeviceComponent(MikanObjectWeakPtr owner)
	: TransformComponent(owner)
{
	m_bWantsCustomRender = true;
}

void VRDeviceComponent::init()
{
	MikanComponent::init();

	// Watch selection changes
	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Push our world transform to all child scene components
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
}

void VRDeviceComponent::customRender()
{
	TextStyle style = getDefaultTextStyle();

	VRDeviceDefinitionPtr anchorDefinition = getVRDeviceDefinition();
	wchar_t wszVRDeviceName[256];
	StringUtils::convertMbsToWcs(anchorDefinition->getComponentName().c_str(), wszVRDeviceName, sizeof(wszVRDeviceName));
	glm::mat4 anchorXform = getWorldTransform();
	glm::vec3 anchorPos(anchorXform[3]);

	glm::vec3 xColor = Colors::DarkRed;
	glm::vec3 yColor = Colors::DarkGreen;
	glm::vec3 zColor = Colors::DarkBlue;
	SelectionComponentPtr selectionComponent = m_selectionComponent.lock();
	if (selectionComponent)
	{
		if (selectionComponent->getIsSelected())
		{
			xColor = Colors::Red;
			yColor = Colors::Green;
			zColor = Colors::Blue;
		}
		else if (selectionComponent->getIsHovered())
		{
			xColor = Colors::LightGreen;
			yColor = Colors::LightGreen;
			zColor = Colors::LightBlue;
		}
	}

	drawTransformedAxes(anchorXform, 0.1f, 0.1f, 0.1f, xColor, yColor, zColor);
	drawTextAtWorldPosition(style, anchorPos, L"%s", wszVRDeviceName);
}