#include "ProjectOutlinerModel.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "ARKitVideoSourceComponent.h"
#include "ARKitVideoSourceSystem.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "BoxStencilSystem.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CEFTextureSourceComponent.h"
#include "CEFTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "LightEnvironmentComponent.h"
#include "LightEnvironmentSystem.h"
#include "LocText.h"
#include "MarkerComponent.h"
#include "MarkerObjectSystem.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MarkerTrackingVolumeSystem.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "ModelStencilSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "ProjectManager.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "QuadStencilSystem.h"
#include "RGBPixelGridComponent.h"
#include "RGBPixelGridSystem.h"
#include "RGBSpotLightComponent.h"
#include "RGBSpotLightSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "SelectionComponent.h"
#include "ShapeComponent.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "StencilComponent.h"
#include "TrackingMountComponent.h"
#include "TrackingMountObjectSystem.h"
#include "TransformComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"
#include "VRTrackingVolumeComponent.h"
#include "VRTrackingVolumeSystem.h"

static bool isSceneActorObject(MikanObjectPtr objectPtr)
{
	return objectPtr->getComponentOfType<AnchorComponent>() != nullptr
		   || objectPtr->getComponentOfType<StencilComponent>() != nullptr
		   || objectPtr->getComponentOfType<ShapeComponent>() != nullptr;
}

void ProjectOutlinerModel::clear()
{
	m_root.reset();
	m_unparentedGroup.reset();
	m_nodeIndex.clear();
}

void ProjectOutlinerModel::rebuild(ProjectManagerPtr projectManager)
{
	clear();

	m_root= std::make_shared<ProjectOutlinerNode>();
	m_root->kind= eOutlinerNodeKind::projectRoot;
	m_root->displayName= locText("project.outlinerRoot");

	if (!projectManager)
		return;

	// Video sources
	if (auto sys= projectManager->getSystemOfType<USBVideoSourceSystem>())
		sys->visitComponents([&](USBVideoSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::videoSource, comp); });
	if (auto sys= projectManager->getSystemOfType<NetworkVideoSourceSystem>())
		sys->visitComponents([&](NetworkVideoSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::videoSource, comp); });
	if (auto sys= projectManager->getSystemOfType<ARKitVideoSourceSystem>())
		sys->visitComponents([&](ARKitVideoSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::videoSource, comp); });

	// Texture sources
	if (auto sys= projectManager->getSystemOfType<ClientTextureSourceSystem>())
		sys->visitComponents([&](ClientTextureSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::textureSource, comp); });
	if (auto sys= projectManager->getSystemOfType<SpoutTextureSourceSystem>())
		sys->visitComponents([&](SpoutTextureSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::textureSource, comp); });
	if (auto sys= projectManager->getSystemOfType<CEFTextureSourceSystem>())
		sys->visitComponents([&](CEFTextureSourceComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::textureSource, comp); });

	// Markers
	if (auto sys= projectManager->getSystemOfType<MarkerObjectSystem>())
		sys->visitComponents([&](MarkerComponentPtr comp)
							 { addComponentNode(m_root, eOutlinerNodeKind::marker, comp); });

	// Tracking volumes, with mounts under the VR volumes that own them
	TrackingMountObjectSystemPtr mountSystem= projectManager->getSystemOfType<TrackingMountObjectSystem>();
	if (auto vrVolumeSystem= projectManager->getSystemOfType<VRTrackingVolumeSystem>())
	{
		vrVolumeSystem->visitComponents(
			[&](VRTrackingVolumeComponentPtr volume)
			{
				ProjectOutlinerNodePtr volumeNode= addComponentNode(m_root, eOutlinerNodeKind::trackingVolume, volume);

				if (mountSystem)
				{
					for (MikanTrackingMountID mountId : volume->getVRTrackingVolumeDefinition()->getTrackingMountIDs())
					{
						if (TrackingMountComponentPtr mount= mountSystem->getTypedComponentById(mountId))
							addComponentNode(volumeNode, eOutlinerNodeKind::trackingMount, mount);
					}
				}
			});
	}
	if (auto markerVolumeSystem= projectManager->getSystemOfType<MarkerTrackingVolumeSystem>())
	{
		markerVolumeSystem->visitComponents([&](MarkerTrackingVolumeComponentPtr volume)
											{ addComponentNode(m_root, eOutlinerNodeKind::trackingVolume, volume); });
	}

	// Stages parent under their volume; one with a missing volume falls back to the root
	if (auto stageSystem= projectManager->getSystemOfType<StageObjectSystem>())
	{
		stageSystem->visitComponents(
			[&](StageComponentPtr stage)
			{
				const MikanTrackingVolumeID volumeId= stage->getStageComponentDefinitionConst()->getTrackingVolumeId();
				ProjectOutlinerNodePtr volumeNode= findNodeByComponentId(volumeId);
				buildStageSubtree(projectManager, stage, volumeNode ? volumeNode : m_root);
			});
	}

	// A scene whose parent stage is gone still needs a row to be repairable from
	if (auto sceneSystem= projectManager->getSystemOfType<SceneObjectSystem>())
	{
		sceneSystem->visitComponents(
			[&](SceneComponentPtr scene)
			{
				if (!findNodeByComponentId(scene->getSceneId()))
					buildSceneSubtree(scene, m_root);
			});
	}

	// Compositors place after every camera and scene row exists: under the
	// camera they view through, else under their owner scene, else at the root
	if (auto compositorSystem= projectManager->getSystemOfType<CompositorObjectSystem>())
	{
		compositorSystem->visitComponents(
			[&](CompositorComponentPtr compositor)
			{
				CompositorDefinitionPtr def= compositor->getCompositorDefinition();
				ProjectOutlinerNodePtr cameraNode= findNodeByComponentId(def->getCameraId());
				ProjectOutlinerNodePtr sceneNode= findNodeByComponentId(def->getOwnerSceneId());
				ProjectOutlinerNodePtr parentNode= cameraNode ? cameraNode : (sceneNode ? sceneNode : m_root);

				addComponentNode(parentNode, eOutlinerNodeKind::compositor, compositor);
			});
	}

	// Scene actors no scene walk reached (legacy unparented shapes, dangling
	// parents) collect in a synthetic group so they stay visible and can be
	// dragged back into a scene. Subtrees start at their root-most orphan: a
	// child whose parent is itself an unlisted actor is reached through it.
	auto addOrphanActor= [&](TransformComponentPtr actor)
	{
		if (findNodeByComponentId(actor->getComponentId()))
			return;

		TransformComponentPtr parentTransform= actor->getParentTransformComponent();
		if (parentTransform && isSceneActorObject(parentTransform->getOwnerObject()))
			return;

		addSceneActorSubtree(actor, getOrCreateUnparentedGroup());
	};
	if (auto sys= projectManager->getSystemOfType<AnchorObjectSystem>())
		sys->visitComponents([&](AnchorComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<QuadStencilSystem>())
		sys->visitComponents([&](QuadStencilComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<BoxStencilSystem>())
		sys->visitComponents([&](BoxStencilComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<ModelStencilSystem>())
		sys->visitComponents([&](ModelStencilComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<QuadShapeSystem>())
		sys->visitComponents([&](QuadShapeComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<BoxShapeSystem>())
		sys->visitComponents([&](BoxShapeComponentPtr comp) { addOrphanActor(comp); });
	if (auto sys= projectManager->getSystemOfType<ModelShapeSystem>())
		sys->visitComponents([&](ModelShapeComponentPtr comp) { addOrphanActor(comp); });
}

void ProjectOutlinerModel::buildStageSubtree(ProjectManagerPtr projectManager, StageComponentPtr stageComponent,
											 ProjectOutlinerNodePtr parentNode)
{
	ProjectOutlinerNodePtr stageNode= addComponentNode(parentNode, eOutlinerNodeKind::stage, stageComponent);
	const MikanStageID stageId= stageComponent->getStageId();

	// Cameras. Their compositors attach in a later pass, once every camera and
	// scene row exists to hang them from.
	if (auto cameraSystem= projectManager->getSystemOfType<CameraObjectSystem>())
	{
		cameraSystem->visitComponents([&](CameraComponentPtr camera)
									  { addComponentNode(stageNode, eOutlinerNodeKind::camera, camera); },
									  [stageId](CameraComponentPtr camera)
									  { return camera->getCameraDefinition()->getOwnerStageId() == stageId; });
	}

	// Stage lights: environment probes, then the authored DMX fixtures
	if (auto lightEnvironmentSystem= projectManager->getSystemOfType<LightEnvironmentSystem>())
	{
		lightEnvironmentSystem->visitComponents(
			[&](LightEnvironmentComponentPtr light)
			{ addComponentNode(stageNode, eOutlinerNodeKind::stageLight, light, "project.envPrefix"); },
			[stageId](LightEnvironmentComponentPtr light)
			{
				StageComponentConstPtr ownerStage= light->getOwnerStageComponent();
				return ownerStage && ownerStage->getComponentId() == stageId;
			});
	}
	if (auto spotLightSystem= projectManager->getSystemOfType<RGBSpotLightSystem>())
	{
		spotLightSystem->visitComponents(
			[&](RGBSpotLightComponentPtr light)
			{ addComponentNode(stageNode, eOutlinerNodeKind::stageLight, light, "project.spotPrefix"); },
			[stageId](RGBSpotLightComponentPtr light)
			{ return light->getDMXFixtureDefinition()->getOwnerStageId() == stageId; });
	}
	if (auto pixelGridSystem= projectManager->getSystemOfType<RGBPixelGridSystem>())
	{
		pixelGridSystem->visitComponents(
			[&](RGBPixelGridComponentPtr light)
			{ addComponentNode(stageNode, eOutlinerNodeKind::stageLight, light, "project.gridPrefix"); },
			[stageId](RGBPixelGridComponentPtr light)
			{ return light->getDMXFixtureDefinition()->getOwnerStageId() == stageId; });
	}

	// Scenes attached to this stage
	if (auto sceneSystem= projectManager->getSystemOfType<SceneObjectSystem>())
	{
		sceneSystem->visitComponents([&](SceneComponentPtr scene) { buildSceneSubtree(scene, stageNode); },
									 [stageId](SceneComponentPtr scene)
									 { return scene->getParentStageId() == stageId; });
	}
}

void ProjectOutlinerModel::buildSceneSubtree(SceneComponentPtr sceneComponent, ProjectOutlinerNodePtr parentNode)
{
	ProjectOutlinerNodePtr sceneNode= addComponentNode(parentNode, eOutlinerNodeKind::scene, sceneComponent);

	// Scene actors via the transform hierarchy
	for (TransformComponentWeakPtr childWeakPtr : sceneComponent->getChildTransformComponents())
	{
		if (TransformComponentPtr child= childWeakPtr.lock())
			addSceneActorSubtree(child, sceneNode);
	}
}

void ProjectOutlinerModel::addSceneActorSubtree(TransformComponentPtr transformComponent,
												ProjectOutlinerNodePtr parentNode)
{
	if (!transformComponent || transformComponent->getWasDisposed())
		return;

	MikanObjectPtr ownerObject= transformComponent->getOwnerObject();
	ProjectOutlinerNodePtr actorNode= parentNode;
	if (ownerObject->getRootComponent() == transformComponent)
	{
		// Only anchor, stencil, and shape objects appear inside a scene subtree
		if (!isSceneActorObject(ownerObject))
			return;

		if (findNodeByComponentId(transformComponent->getComponentId()))
			return;

		actorNode= addComponentNode(parentNode, eOutlinerNodeKind::sceneActor, transformComponent);
	}

	for (TransformComponentWeakPtr childWeakPtr : transformComponent->getChildTransformComponents())
	{
		if (TransformComponentPtr child= childWeakPtr.lock())
			addSceneActorSubtree(child, actorNode);
	}
}

ProjectOutlinerNodePtr ProjectOutlinerModel::addComponentNode(ProjectOutlinerNodePtr parentNode, eOutlinerNodeKind kind,
															  MikanComponentPtr component, const char* namePrefixKey)
{
	ProjectOutlinerNodePtr node= std::make_shared<ProjectOutlinerNode>();
	node->kind= kind;
	node->componentId= component->getComponentId();
	node->componentClassName= component->getComponentClassName();
	node->component= component;

	if (MikanObjectPtr ownerObject= component->getOwnerObject())
	{
		if (MikanObjectSystemPtr ownerSystem= ownerObject->getOwnerSystem())
			node->systemClassName= ownerSystem->getObjectSystemClassName();
		node->selection= ownerObject->getComponentOfType<SelectionComponent>();
	}

	std::string name= component->getName();
	if (name.empty())
		name= locText("project.noName");
	node->displayName= namePrefixKey ? (std::string(locText(namePrefixKey)) + name) : name;

	node->parent= parentNode;
	parentNode->children.push_back(node);
	m_nodeIndex[node->componentId]= node;

	return node;
}

ProjectOutlinerNodePtr ProjectOutlinerModel::getOrCreateUnparentedGroup()
{
	if (!m_unparentedGroup)
	{
		m_unparentedGroup= std::make_shared<ProjectOutlinerNode>();
		m_unparentedGroup->kind= eOutlinerNodeKind::unparentedGroup;
		m_unparentedGroup->displayName= locText("project.outlinerUnparented");
		m_unparentedGroup->parent= m_root;
		m_root->children.push_back(m_unparentedGroup);
	}

	return m_unparentedGroup;
}

ProjectOutlinerNodePtr ProjectOutlinerModel::findNodeByComponentId(int componentId) const
{
	auto it= m_nodeIndex.find(componentId);
	return (it != m_nodeIndex.end()) ? it->second.lock() : nullptr;
}

ProjectOutlinerNodePtr ProjectOutlinerModel::findNodeBySelection(SelectionComponentConstPtr selection) const
{
	if (!selection)
		return nullptr;

	for (const auto& kvpair : m_nodeIndex)
	{
		ProjectOutlinerNodePtr node= kvpair.second.lock();
		if (node && node->selection.lock().get() == selection.get())
			return node;
	}

	return nullptr;
}

ProjectOutlinerNodePtr ProjectOutlinerModel::findOwningSceneNode(ProjectOutlinerNodePtr node) const
{
	while (node && node->kind != eOutlinerNodeKind::scene)
	{
		node= node->parent.lock();
	}

	return node;
}
