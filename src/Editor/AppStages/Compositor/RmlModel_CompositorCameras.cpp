#include "RmlModel_CompositorCameras.h"
#include "GlFrameCompositor.h"
#include "StringUtils.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_CompositorCameras::init(
	Rml::Context* rmlContext,
	const GlFrameCompositor* compositor)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "compositor_video");
	if (!constructor)
		return false;

	// Register Data Model Fields
	constructor.Bind("camera_names", &m_cameraNames);

	// Bind data model callbacks

	return true;
}

void RmlModel_CompositorCameras::dispose()
{
	RmlModel::dispose();
}