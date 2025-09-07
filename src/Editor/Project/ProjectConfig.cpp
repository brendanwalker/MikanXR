// -- includes -----
#include "AnchorObjectSystem.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "MathUtility.h"
#include "MarkerObjectSystem.h"
#include "ProjectConfig.h"
#include "ProjectConfigConstants.h"
#include "PathUtils.h"
#include "StencilObjectSystem.h"
#include "SceneObjectSystem.h"
#include "StageObjectSystem.h"
#include "StringUtils.h"
#include "SinglecastDelegate.h"
#include "TrackingSystemsConfig.h"
#include "TrackingMountObjectSystem.h"
#include "VideoSourceSystemConfig.h"
#include "VRObjectSystem.h"

// -- Profile Config
const std::string ProjectConfig::k_renderOriginFlagPropertyId= "renderOrigin";

ProjectConfig::ProjectConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
	// Compositor
	, compositorScriptFilePath("")
{
	editorConfig = std::make_shared<EditorObjectSystemConfig>("editor");
	addChildConfig(editorConfig);

	sceneConfig = std::make_shared<SceneObjectSystemConfig>("scenes");
	addChildConfig(sceneConfig);

	cameraConfig = std::make_shared<CameraObjectSystemConfig>("cameras");
	addChildConfig(cameraConfig);

	compositorConfig = std::make_shared<CompositorObjectSystemConfig>("compositors");
	addChildConfig(compositorConfig);

	stageConfig = std::make_shared<StageObjectSystemConfig>("stages");
	addChildConfig(stageConfig);

	anchorConfig= std::make_shared<AnchorObjectSystemConfig>("anchors");
	addChildConfig(anchorConfig);

	stencilConfig= std::make_shared<StencilObjectSystemConfig>("stencils");
	addChildConfig(stencilConfig);

	markerSystemConfig = std::make_shared<MarkerObjectSystemConfig>("marker_system");
	addChildConfig(markerSystemConfig);

	trackingSystemsConfig = std::make_shared<TrackingSystemsConfig>("tracking_systems");
	addChildConfig(trackingSystemsConfig);

	trackingMountSystemConfig = std::make_shared<TrackingMountObjectSystemConfig>("tracking_mount_system");
	addChildConfig(trackingMountSystemConfig);

	videoSourceSystemConfig = std::make_shared<VideoSourceSystemConfig>("video_source_system");
	addChildConfig(videoSourceSystemConfig);

	vrObjectConfig= std::make_shared<VRObjectSystemConfig>("vr_objects");
	addChildConfig(vrObjectConfig);
};

configuru::Config ProjectConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	// Compositor
	pt["compositorScript"]= compositorScriptFilePath.string();
	// Renderer Flags
	pt[k_renderOriginFlagPropertyId]= m_bRenderOrigin;

	// Write the editor system config
	pt[editorConfig->getConfigName()] = editorConfig->writeToJSON();

	// Write the scene system config
	pt[sceneConfig->getConfigName()] = sceneConfig->writeToJSON();

	// Write the camera system config
	pt[cameraConfig->getConfigName()] = cameraConfig->writeToJSON();

	// Write the compositor system config
	pt[compositorConfig->getConfigName()] = compositorConfig->writeToJSON();

	// Write the stage system config
	pt[stageConfig->getConfigName()] = stageConfig->writeToJSON();

	// Write the anchor system config
	pt[anchorConfig->getConfigName()]= anchorConfig->writeToJSON();

	// Write the marker system config
	pt[markerSystemConfig->getConfigName()] = markerSystemConfig->writeToJSON();

	// Write the stencil system config
	pt[stencilConfig->getConfigName()]= stencilConfig->writeToJSON();

	// Write the tracking systems config
	pt[trackingSystemsConfig->getConfigName()] = trackingSystemsConfig->writeToJSON();

	// Write the tracking mount system config
	pt[trackingMountSystemConfig->getConfigName()] = trackingMountSystemConfig->writeToJSON();

	// Write the video source system config
	pt[videoSourceSystemConfig->getConfigName()] = videoSourceSystemConfig->writeToJSON();

	// Write the vr object system config
	pt[vrObjectConfig->getConfigName()] = vrObjectConfig->writeToJSON();

	return pt;
}

void ProjectConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);


	m_bRenderOrigin = pt.get_or<bool>(k_renderOriginFlagPropertyId, m_bRenderOrigin);

	// Read the editor system config
	if (pt.has_key(editorConfig->getConfigName()))
	{
		editorConfig->readFromJSON(pt[editorConfig->getConfigName()]);
	}

	// Read the scene system config
	if (pt.has_key(sceneConfig->getConfigName()))
	{
		sceneConfig->readFromJSON(pt[sceneConfig->getConfigName()]);
	}

	// Read the camera system config
	if (pt.has_key(cameraConfig->getConfigName()))
	{
		cameraConfig->readFromJSON(pt[cameraConfig->getConfigName()]);
	}

	// Read the compositor system config
	if (pt.has_key(compositorConfig->getConfigName()))
	{
		compositorConfig->readFromJSON(pt[compositorConfig->getConfigName()]);
	}

	// Read the stage system config
	if (pt.has_key(stageConfig->getConfigName()))
	{
		stageConfig->readFromJSON(pt[stageConfig->getConfigName()]);
	}

	// Read the anchor system config
	if (pt.has_key(anchorConfig->getConfigName()))
	{
		anchorConfig->readFromJSON(pt[anchorConfig->getConfigName()]);
	}

	// Read the marker system config
	if (pt.has_key(markerSystemConfig->getConfigName()))
	{
		markerSystemConfig->readFromJSON(pt[markerSystemConfig->getConfigName()]);
	}

	// Read the stencil system config
	if (pt.has_key(stencilConfig->getConfigName()))
	{
		stencilConfig->readFromJSON(pt[stencilConfig->getConfigName()]);
	}

	// Read the tracking systems config
	if (pt.has_key(trackingSystemsConfig->getConfigName()))
	{
		trackingSystemsConfig->readFromJSON(pt[trackingSystemsConfig->getConfigName()]);
	}

	// Read the tracking mount system config
	if (pt.has_key(trackingMountSystemConfig->getConfigName()))
	{
		trackingMountSystemConfig->readFromJSON(pt[trackingMountSystemConfig->getConfigName()]);
	}

	// Read the video source system config
	if (pt.has_key(videoSourceSystemConfig->getConfigName()))
	{
		videoSourceSystemConfig->readFromJSON(pt[videoSourceSystemConfig->getConfigName()]);
	}

	// Read the vr object system config
	if (pt.has_key(vrObjectConfig->getConfigName()))
	{
		vrObjectConfig->readFromJSON(pt[vrObjectConfig->getConfigName()]);
	}

	// Compositor
	compositorScriptFilePath = pt.get_or<std::string>("compositorScript", compositorScriptFilePath.string());
}


void ProjectConfig::setRenderOriginFlag(bool flag)
{
	if (m_bRenderOrigin != flag)
	{
		m_bRenderOrigin = flag;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_renderOriginFlagPropertyId));
	}
}