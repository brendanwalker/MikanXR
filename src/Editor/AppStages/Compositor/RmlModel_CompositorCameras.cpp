#include "RmlModel_CompositorCameras.h"
#include "GlFrameCompositor.h"
#include "Shared/RmlDataBinding_VRDeviceList.h"
#include "StringUtils.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_CompositorCameras::RmlModel_CompositorCameras()
	: m_vrDeviceBinding(std::make_shared<RmlDataBinding_VRDeviceList>())
{}

bool RmlModel_CompositorCameras::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_video");
	if (!constructor)
		return false;

	// Bind vr devices to the data model
	if (!m_vrDeviceBinding->init(constructor))
		return false;

	// Register Data Model Fields
	constructor.Bind("camera_vr_device_path", &m_cameraVRDevicePath);
	constructor.Bind("camera_names", &m_cameraNames);

	// Bind data model callbacks

	return true;
}

void RmlModel_CompositorCameras::dispose()
{
	m_vrDeviceBinding->dispose();
	RmlModel::dispose();
}