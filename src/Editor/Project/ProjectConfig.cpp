// -- includes -----
#include "AnchorObjectSystem.h"
#include "BoxStencilSystem.h"
#include "CameraObjectSystem.h"
#include "ClientTextureSourceSystem.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "MathUtility.h"
#include "MarkerObjectSystem.h"
#include "MarkerTrackingVolumeSystem.h"
#include "ModelStencilSystem.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectConfig.h"
#include "ProjectConfigConstants.h"
#include "PathUtils.h"
#include "QuadStencilSystem.h"
#include "SceneObjectSystem.h"
#include "StageObjectSystem.h"
#include "SpoutTextureSourceSystem.h"
#include "StringUtils.h"
#include "SinglecastDelegate.h"
#include "TrackingMountObjectSystem.h"
#include "USBVideoSourceSystem.h"
#include "VRObjectSystem.h"
#include "VRTrackingVolumeSystem.h"

// -- Profile Config
const std::string ProjectConfig::k_renderOriginFlagPropertyId= "renderOrigin";

ProjectConfig::ProjectConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
{
	anchorConfig = addTypedDefinition<AnchorObjectSystemDefinition, AnchorObjectSystem>();
	boxStencilSystemDefinition = addTypedDefinition<BoxStencilSystemDefinition, BoxStencilSystem>();
	cameraConfig = addTypedDefinition<CameraObjectSystemDefinition, CameraObjectSystem>();
	clientConfig = addTypedDefinition<ClientTextureSourceSystemConfig, ClientTextureSourceSystem>();
	compositorConfig = addTypedDefinition<CompositorObjectSystemConfig, CompositorObjectSystem>();
	editorConfig = addTypedDefinition<EditorObjectSystemConfig, EditorObjectSystem>();
	markerSystemDefinition = addTypedDefinition<MarkerObjectSystemDefinition, MarkerObjectSystem>();
	markerTrackingVolumeConfig = addTypedDefinition<MarkerTrackingVolumeSystemDefinition, MarkerTrackingVolumeSystem>();
	modelStencilSystemDefinition = addTypedDefinition<ModelStencilSystemDefinition, ModelStencilSystem>();
	quadStencilSystemDefinition = addTypedDefinition<QuadStencilSystemDefinition, QuadStencilSystem>();
	sceneConfig = addTypedDefinition<SceneObjectSystemDefinition, SceneObjectSystem>();
	stageConfig = addTypedDefinition<StageObjectSystemDefinition, StageObjectSystem>();
	spoutConfig = addTypedDefinition<SpoutTextureSourceSystemConfig, SpoutTextureSourceSystem>();
	trackingMountSystemConfig = addTypedDefinition<TrackingMountObjectSystemConfig, TrackingMountObjectSystem>();
	networkVideoSourceSystemConfig = addTypedDefinition<NetworkVideoSourceSystemConfig, NetworkVideoSourceSystem>();
	usbVideoSourceSystemConfig = addTypedDefinition<USBVideoSourceSystemConfig, USBVideoSourceSystem>();
	vrObjectConfig= addTypedDefinition<VRObjectSystemConfig, VRObjectSystem>();
	vrTrackingVolumeConfig = addTypedDefinition<VRTrackingVolumeSystemDefinition, VRTrackingVolumeSystem>();
};

configuru::Config ProjectConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	// Renderer Flags
	pt[k_renderOriginFlagPropertyId]= m_bRenderOrigin;

	// Write out all child configs
	for (const auto& childConfigPtr : m_childConfigs)
	{
		pt[childConfigPtr->getConfigName()] = childConfigPtr->writeToJSON();
	}

	return pt;
}

void ProjectConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_bRenderOrigin = pt.get_or<bool>(k_renderOriginFlagPropertyId, m_bRenderOrigin);

	// Read all child configs
	for (const auto& childConfigPtr : m_childConfigs)
	{
		if (pt.has_key(childConfigPtr->getConfigName()))
		{
			const configuru::Config& child_pt = pt[childConfigPtr->getConfigName()];

			childConfigPtr->readFromJSON(child_pt);
		}
	}
}

void ProjectConfig::setRenderOriginFlag(bool flag)
{
	if (m_bRenderOrigin != flag)
	{
		m_bRenderOrigin = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderOriginFlagPropertyId));
	}
}

MikanObjectSystemDefinitionPtr ProjectConfig::getDefinitionForSystem(MikanObjectSystemPtr systemPtr) const
{
	const std::string systemClassName = systemPtr->getObjectSystemClassName();
	std::shared_ptr<CommonConfig> systemConfig= getChildConfig(systemClassName);

	if (systemConfig)
	{
		return std::static_pointer_cast<MikanObjectSystemDefinition>(systemConfig);
	}

	return nullptr;
}