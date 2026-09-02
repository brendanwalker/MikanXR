#include "GuiPanel_ProjectOutliner.h"
#include "AnchorComponent.h"
#include "ARKitVideoSourceComponent.h"
#include "BoxShapeComponent.h"
#include "BoxStencilComponent.h"
#include "CameraComponent.h"
#include "CEFTextureSourceComponent.h"
#include "ClientTextureSourceComponent.h"
#include "CommonConfig.h"
#include "CompositorComponent.h"
#include "DMXFixtureComponent.h"
#include "EditorObjectSystem.h"
#include "IEditorWindow.h"
#include "LightEnvironmentComponent.h"
#include "LocText.h"
#include "MarkerComponent.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "MikanObjectSystem.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiScopedDragDropSource.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "ModalConfirm/ModalDialog_Confirm.h"
#include "ModelShapeComponent.h"
#include "ModelStencilComponent.h"
#include "NetworkVideoSourceComponent.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "Project/ProjectOutlinerActions.h"
#include "ProjectManager.h"
#include "QuadShapeComponent.h"
#include "QuadStencilComponent.h"
#include "RGBPixelGridComponent.h"
#include "RGBSpotLightComponent.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "SelectionComponent.h"
#include "Shared/GuiPanel_AnchorComponent.h"
#include "Shared/GuiPanel_ARKitVideoSourceComponent.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "Shared/GuiPanel_CEFTextureSourceComponent.h"
#include "Shared/GuiPanel_ClientTextureSourceComponent.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "Shared/GuiPanel_LightEnvironmentComponent.h"
#include "Shared/GuiPanel_MarkerComponent.h"
#include "Shared/GuiPanel_MarkerTrackingVolumeComponent.h"
#include "Shared/GuiPanel_NetworkVideoSourceComponent.h"
#include "Shared/GuiPanel_RGBPixelGridComponent.h"
#include "Shared/GuiPanel_RGBSpotLightComponent.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "Shared/GuiPanel_ShapeComponent.h"
#include "Shared/GuiPanel_SpoutTextureSourceComponent.h"
#include "Shared/GuiPanel_StageComponent.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "Shared/GuiPanel_USBVideoSourceComponent.h"
#include "Shared/GuiPanel_VRTrackingVolumeComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "StageComponent.h"
#include "TrackingMountComponent.h"
#include "TransactionHistory.h"
#include "TransformComponent.h"
#include "USBVideoSourceComponent.h"
#include "VRTrackingVolumeComponent.h"

#include "imgui.h"

#include <algorithm>

// One payload type for all seven scene actor classes: the payload itself is
// always a TransformComponentPtr
static const char* k_outlinerActorPayloadType= "OutlinerSceneActor";

GuiPanel_ProjectOutliner::GuiPanel_ProjectOutliner(AppStage* ownerAppStage)
	: GuiPanel(ownerAppStage)
{
}

bool GuiPanel_ProjectOutliner::init(ProjectGuiPanelContext* context)
{
	m_context= context;
	m_projectManager= m_ownerAppStage->getProjectManager();
	m_editorSystem= m_ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();
	m_sceneSystem= m_ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");
	m_outlinerGuiStyle= getGuiStyleManager()->getStyle("project_outliner");

	if (EditorObjectSystemPtr editorSystem= m_editorSystem.lock())
	{
		editorSystem->OnSelectionChanged+= MakeDelegate(this, &GuiPanel_ProjectOutliner::onSelectionChanged);
	}

	subscribeToSystems();
	m_bTreeDirty= true;

	return true;
}

void GuiPanel_ProjectOutliner::dispose()
{
	if (EditorObjectSystemPtr editorSystem= m_editorSystem.lock())
	{
		editorSystem->OnSelectionChanged-= MakeDelegate(this, &GuiPanel_ProjectOutliner::onSelectionChanged);
	}

	unsubscribeFromSystems();
	m_model.clear();

	GuiPanel::dispose();
}

void GuiPanel_ProjectOutliner::onGui()
{
	rebuildIfDirty();

	drawTree();

	ImGui::Separator();

	drawSelectedNodeActions(getSelectedNode());
}

void GuiPanel_ProjectOutliner::rebuildIfDirty()
{
	if (!m_bTreeDirty)
		return;

	m_bTreeDirty= false;
	m_model.rebuild(m_projectManager.lock());

	if (m_pendingSelectComponentId != INVALID_MIKAN_ID)
	{
		ProjectOutlinerNodePtr newNode= m_model.findNodeByComponentId(m_pendingSelectComponentId);
		m_pendingSelectComponentId= INVALID_MIKAN_ID;
		if (newNode)
		{
			setSelectedNode(newNode, true);
			return;
		}
	}

	// Re-resolve the selection against the rebuilt tree: rebind the panel to the
	// fresh component, or fall back to the root when the object is gone
	if (m_selectedComponentId != INVALID_MIKAN_ID)
	{
		ProjectOutlinerNodePtr selectedNode= m_model.findNodeByComponentId(m_selectedComponentId);
		setSelectedNode(selectedNode, false);
	}
	else if (m_selectedKind != eOutlinerNodeKind::projectRoot
			 && !m_model.findFolderNode(m_selectedKind, m_selectedOwnerId))
	{
		// A selected per-stage folder whose stage was deleted falls back to the root
		setSelectedNode(nullptr, false);
	}
}

// -- Tree drawing ----
void GuiPanel_ProjectOutliner::drawTree()
{
	const float treeHeight= std::max(150.f, ImGui::GetContentRegionAvail().y * 0.4f);
	if (ImGui::BeginChild("##OutlinerTree", ImVec2(0.f, treeHeight)))
	{
		if (ProjectOutlinerNodePtr rootNode= m_model.getRoot())
		{
			drawNode(rootNode);
		}
	}
	ImGui::EndChild();
}

void GuiPanel_ProjectOutliner::drawNode(ProjectOutlinerNodePtr node)
{
	ImGuiTreeNodeFlags flags=
		ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (node->children.empty())
		flags|= ImGuiTreeNodeFlags_Leaf;

	switch (node->kind)
	{
	case eOutlinerNodeKind::projectRoot:
	case eOutlinerNodeKind::folderSources:
	case eOutlinerNodeKind::folderMarkers:
	case eOutlinerNodeKind::folderTrackingVolumes:
	case eOutlinerNodeKind::folderCameras:
	case eOutlinerNodeKind::folderLights:
	case eOutlinerNodeKind::folderScenes:
	case eOutlinerNodeKind::trackingVolume:
	case eOutlinerNodeKind::stage:
	case eOutlinerNodeKind::scene:
		flags|= ImGuiTreeNodeFlags_DefaultOpen;
		break;
	default:
		break;
	}

	// Synthetic rows (root and folders) carry no component id, so they select
	// by kind plus owner id
	const bool bIsSyntheticSelectable=
		node->kind == eOutlinerNodeKind::projectRoot || node->kind == eOutlinerNodeKind::folderSources
		|| node->kind == eOutlinerNodeKind::folderMarkers || node->kind == eOutlinerNodeKind::folderTrackingVolumes
		|| node->kind == eOutlinerNodeKind::folderCameras || node->kind == eOutlinerNodeKind::folderLights
		|| node->kind == eOutlinerNodeKind::folderScenes;
	const bool bIsSelected= (node->componentId != INVALID_MIKAN_ID && node->componentId == m_selectedComponentId)
							|| (bIsSyntheticSelectable && m_selectedComponentId == INVALID_MIKAN_ID
								&& node->kind == m_selectedKind && node->ownerId == m_selectedOwnerId);
	if (bIsSelected)
		flags|= ImGuiTreeNodeFlags_Selected;

	// A pending scroll-to-selection opens the collapsed path above the target row
	if (m_bScrollToSelection && node->componentId != INVALID_MIKAN_ID
		&& m_scrollOpenPathIds.find(node->componentId) != m_scrollOpenPathIds.end())
	{
		ImGui::SetNextItemOpen(true);
	}

	const std::string label=
		node->displayName + "##node" + std::to_string((int)node->kind) + "_" + std::to_string(node->componentId);

	bool bOpen= false;
	if (isActiveHighlightNode(node))
	{
		MkGuiScopedStyle activeStyle(m_outlinerGuiStyle);
		bOpen= ImGui::TreeNodeEx(label.c_str(), flags);
	}
	else
	{
		bOpen= ImGui::TreeNodeEx(label.c_str(), flags);
	}

	if (bIsSelected && m_bScrollToSelection)
	{
		ImGui::SetScrollHereY(0.5f);
		m_bScrollToSelection= false;
		m_scrollOpenPathIds.clear();
	}

	// The unparented tray is the one row that is not selectable
	const bool bIsSelectable= node->componentId != INVALID_MIKAN_ID || bIsSyntheticSelectable;
	if (bIsSelectable && ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
	{
		ProjectOutlinerNodePtr clickedNode= node;
		addDeferredGuiEvent([this, clickedNode]() { setSelectedNode(clickedNode, true); });
	}

	handleNodeDragDrop(node);

	if (bOpen)
	{
		for (ProjectOutlinerNodePtr childNode : node->children)
		{
			drawNode(childNode);
		}
		ImGui::TreePop();
	}
}

void GuiPanel_ProjectOutliner::handleNodeDragDrop(ProjectOutlinerNodePtr node)
{
	// Drag sources: scene actors only. Nothing above a scene moves by drag.
	if (node->kind == eOutlinerNodeKind::sceneActor)
	{
		TransformComponentPtr transformComponent= std::dynamic_pointer_cast<TransformComponent>(node->component.lock());
		if (transformComponent)
		{
			MkGuiScopedDragDropSource dragSource;
			if (dragSource)
			{
				ImGui::SetDragDropPayload(k_outlinerActorPayloadType, &transformComponent,
										  sizeof(TransformComponentPtr));
				ImGui::TextUnformatted(node->displayName.c_str());
			}
		}
	}

	// Drop targets: a scene (cross-scene moves included) or another scene actor
	if (node->kind == eOutlinerNodeKind::scene || node->kind == eOutlinerNodeKind::sceneActor)
	{
		TransformComponentPtr droppedComponent=
			MkGui::receiveTypedDragDropPayload<TransformComponent>(k_outlinerActorPayloadType);
		if (droppedComponent)
		{
			TransformComponentPtr newParent= std::dynamic_pointer_cast<TransformComponent>(node->component.lock());
			addDeferredGuiEvent(
				[this, droppedComponent, newParent]()
				{
					TransactionHistory* transactionHistory= m_ownerAppStage->getOwnerWindow()->getTransactionHistory();
					if (transactionHistory)
						transactionHistory->beginGesture("outliner_reparent");

					ProjectOutlinerActions::reparentSceneActor(droppedComponent, newParent);

					if (transactionHistory)
						transactionHistory->endGesture();
				});
		}
	}
}

// -- Selected-node action strip ----
void GuiPanel_ProjectOutliner::drawSelectedNodeActions(ProjectOutlinerNodePtr selectedNode)
{
	if (!selectedNode)
		return;

	switch (selectedNode->kind)
	{
	case eOutlinerNodeKind::folderSources:
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddUSBSource", "add_usb_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addUSBVideoSource(pm); });
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddNetworkSource", "add_network_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addNetworkVideoSource(pm); });
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddARKitSource", "add_arkit_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addARKitVideoSource(pm); });

		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddClientSource", "add_client_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addClientTextureSource(pm); });
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddSpoutSource", "add_spout_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addSpoutTextureSource(pm); });
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddCEFSource", "add_cef_source"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addCEFTextureSource(pm); });
		break;
	case eOutlinerNodeKind::folderMarkers:
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddMarker", "add_marker"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addMarker(pm); });
		break;
	case eOutlinerNodeKind::folderTrackingVolumes:
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddMarkerTracking", "add_marker_tracking"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addMarkerTrackingVolume(pm); });
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddVRTracking", "add_vr_tracking"))
			deferAddAction([](ProjectManagerPtr pm) { return ProjectOutlinerActions::addVRTrackingVolume(pm); });
		break;
	case eOutlinerNodeKind::trackingVolume:
	{
		const int volumeId= selectedNode->componentId;
		if (ImGui::Button(locLabel("project.outlinerAddStage")))
		{
			deferAddAction([volumeId](ProjectManagerPtr pm) { return ProjectOutlinerActions::addStage(pm, volumeId); });
		}
		if (selectedNode->componentClassName == VRTrackingVolumeComponent::k_componentClassName)
		{
			ImGui::SameLine();
			if (ImGui::Button(locLabel("project.outlinerAddMount")))
			{
				deferAddAction([volumeId](ProjectManagerPtr pm)
							   { return ProjectOutlinerActions::addTrackingMount(pm, volumeId); });
			}
		}
		break;
	}
	case eOutlinerNodeKind::folderCameras:
	{
		const int stageId= selectedNode->ownerId;
		if (ImGui::Button(locLabel("project.outlinerAddCamera")))
		{
			deferAddAction([stageId](ProjectManagerPtr pm) { return ProjectOutlinerActions::addCamera(pm, stageId); });
		}
		break;
	}
	case eOutlinerNodeKind::folderLights:
	{
		const int stageId= selectedNode->ownerId;
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddSpotLight", "add_spot_light"))
		{
			deferAddAction([stageId](ProjectManagerPtr pm)
						   { return ProjectOutlinerActions::addSpotLight(pm, stageId); });
		}
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddPixelGrid", "add_pixel_grid"))
		{
			deferAddAction([stageId](ProjectManagerPtr pm)
						   { return ProjectOutlinerActions::addPixelGrid(pm, stageId); });
		}
		break;
	}
	case eOutlinerNodeKind::folderScenes:
	{
		const int stageId= selectedNode->ownerId;
		if (ImGui::Button(locLabel("project.outlinerAddScene")))
		{
			deferAddAction([stageId](ProjectManagerPtr pm) { return ProjectOutlinerActions::addScene(pm, stageId); });
		}
		break;
	}
	case eOutlinerNodeKind::scene:
	{
		const int sceneId= selectedNode->componentId;
		if (ImGui::Button(locLabel("project.outlinerAddCompositor")))
		{
			deferAddAction([sceneId](ProjectManagerPtr pm)
						   { return ProjectOutlinerActions::addCompositor(pm, sceneId); });
		}
		drawSceneActorAddButtons(selectedNode);
		break;
	}
	case eOutlinerNodeKind::sceneActor:
		drawSceneActorAddButtons(selectedNode);
		break;
	default:
		break;
	}

	// Environment probes are camera-owned, so everything else gets a delete button
	const bool bCanDelete= selectedNode->componentId != INVALID_MIKAN_ID
						   && selectedNode->componentClassName != LightEnvironmentComponent::k_componentClassName;
	if (bCanDelete)
	{
		if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerDelete", "delete_component"))
		{
			requestDeleteNode(selectedNode);
		}
	}

	ImGui::Separator();

	drawComponentPanelForNode(selectedNode);
}

void GuiPanel_ProjectOutliner::drawSceneActorAddButtons(ProjectOutlinerNodePtr selectedNode)
{
	const int parentTransformId= getAddParentTransformId(selectedNode);
	if (parentTransformId == INVALID_MIKAN_ID)
		return;

	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddAnchor", "add_anchor"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addAnchor(pm, parentTransformId); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddQuadStencil", "add_quad_stencil"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addStencil(pm, eStencilType::quad, parentTransformId); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddBoxStencil", "add_box_stencil"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addStencil(pm, eStencilType::box, parentTransformId); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddModelStencil", "add_model_stencil"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addStencil(pm, eStencilType::model, parentTransformId); });
	}

	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddQuadShape", "add_quad_shape"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addShape(pm, eShapeType::quad, parentTransformId); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddBoxShape", "add_box_shape"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addShape(pm, eShapeType::box, parentTransformId); });
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "outlinerAddModelShape", "add_model_shape"))
	{
		deferAddAction([parentTransformId](ProjectManagerPtr pm)
					   { return ProjectOutlinerActions::addShape(pm, eShapeType::model, parentTransformId); });
	}
}

int GuiPanel_ProjectOutliner::getAddParentTransformId(ProjectOutlinerNodePtr selectedNode) const
{
	if (!selectedNode)
		return INVALID_MIKAN_ID;

	if (selectedNode->kind == eOutlinerNodeKind::scene || selectedNode->kind == eOutlinerNodeKind::sceneActor)
		return selectedNode->componentId;

	return INVALID_MIKAN_ID;
}

void GuiPanel_ProjectOutliner::deferAddAction(std::function<int(ProjectManagerPtr)> addAction)
{
	addDeferredGuiEvent(
		[this, addAction]()
		{
			ProjectManagerPtr projectManager= m_projectManager.lock();
			if (!projectManager)
				return;

			const int newComponentId= addAction(projectManager);
			if (newComponentId != INVALID_MIKAN_ID)
			{
				m_pendingSelectComponentId= newComponentId;
				markTreeDirty();
			}
		});
}

void GuiPanel_ProjectOutliner::requestDeleteNode(ProjectOutlinerNodePtr node)
{
	const int objectCount= ProjectOutlinerActions::countSubtreeObjects(node);
	const std::string question=
		objectCount > 1 ? locFormat("project.outlinerDeleteManyFmt", node->displayName.c_str(), objectCount - 1)
						: locFormat("project.outlinerDeleteOneFmt", node->displayName.c_str());

	// The captured node keeps its subtree snapshot alive across the modal
	ModalDialog_Confirm::confirmQuestion(m_ownerAppStage, locText("project.outlinerDeleteConfirmTitle"), question,
										 [this, node]()
										 {
											 addDeferredGuiEvent(
												 [this, node]()
												 {
													 if (ProjectManagerPtr projectManager= m_projectManager.lock())
													 {
														 ProjectOutlinerActions::deleteSubtree(projectManager, node);
													 }
												 });
										 });
}

// -- Selection plumbing ----
ProjectOutlinerNodePtr GuiPanel_ProjectOutliner::getSelectedNode() const
{
	if (m_selectedComponentId != INVALID_MIKAN_ID)
	{
		if (ProjectOutlinerNodePtr node= m_model.findNodeByComponentId(m_selectedComponentId))
			return node;
	}

	if (ProjectOutlinerNodePtr folderNode= m_model.findFolderNode(m_selectedKind, m_selectedOwnerId))
		return folderNode;

	return m_model.getRoot();
}

void GuiPanel_ProjectOutliner::setSelectedNode(ProjectOutlinerNodePtr node, bool bPushViewportSelection)
{
	m_selectedComponentId= node ? node->componentId : INVALID_MIKAN_ID;
	m_selectedKind= node ? node->kind : eOutlinerNodeKind::projectRoot;
	m_selectedOwnerId= node ? node->ownerId : INVALID_MIKAN_ID;

	clearComponentPanels();
	if (node)
	{
		if (MikanComponentPtr component= node->component.lock())
		{
			if (GuiPanel_MikanComponent* panel= getPanelForComponentClass(node->componentClassName))
				panel->setComponent(component);
		}
	}

	if (bPushViewportSelection)
	{
		if (EditorObjectSystemPtr editorSystem= m_editorSystem.lock())
			editorSystem->setSelection(node ? node->selection.lock() : nullptr);
	}
}

GuiPanel_MikanComponent* GuiPanel_ProjectOutliner::getPanelForComponentClass(
	const std::string& componentClassName) const
{
	if (componentClassName == USBVideoSourceComponent::k_componentClassName)
		return m_context->getUSBVideoSourcePanel();
	if (componentClassName == NetworkVideoSourceComponent::k_componentClassName)
		return m_context->getNetworkVideoSourcePanel();
	if (componentClassName == ARKitVideoSourceComponent::k_componentClassName)
		return m_context->getARKitVideoSourcePanel();
	if (componentClassName == ClientTextureSourceComponent::k_componentClassName)
		return m_context->getClientTextureSourcePanel();
	if (componentClassName == SpoutTextureSourceComponent::k_componentClassName)
		return m_context->getSpoutTextureSourcePanel();
	if (componentClassName == CEFTextureSourceComponent::k_componentClassName)
		return m_context->getCEFTextureSourcePanel();
	if (componentClassName == MarkerComponent::k_componentClassName)
		return m_context->getMarkerPanel();
	if (componentClassName == VRTrackingVolumeComponent::k_componentClassName)
		return m_context->getVRTrackingVolumePanel();
	if (componentClassName == MarkerTrackingVolumeComponent::k_componentClassName)
		return m_context->getMarkerTrackingVolumePanel();
	if (componentClassName == TrackingMountComponent::k_componentClassName)
		return m_context->getTrackingMountPanel();
	if (componentClassName == StageComponent::k_componentClassName)
		return m_context->getStagePanel();
	if (componentClassName == CameraComponent::k_componentClassName)
		return m_context->getCameraPanel();
	if (componentClassName == CompositorComponent::k_componentClassName)
		return m_context->getCompositorPanel();
	if (componentClassName == SceneComponent::k_componentClassName)
		return m_context->getScenePanel();
	if (componentClassName == LightEnvironmentComponent::k_componentClassName)
		return m_context->getLightEnvironmentPanel();
	if (componentClassName == RGBSpotLightComponent::k_componentClassName)
		return m_context->getSpotLightPanel();
	if (componentClassName == RGBPixelGridComponent::k_componentClassName)
		return m_context->getPixelGridPanel();
	if (componentClassName == AnchorComponent::k_componentClassName)
		return m_context->getAnchorPanel();
	if (componentClassName == QuadStencilComponent::k_componentClassName)
		return m_context->getQuadStencilPanel();
	if (componentClassName == BoxStencilComponent::k_componentClassName)
		return m_context->getBoxStencilPanel();
	if (componentClassName == ModelStencilComponent::k_componentClassName)
		return m_context->getModelStencilPanel();
	if (componentClassName == QuadShapeComponent::k_componentClassName)
		return m_context->getQuadShapePanel();
	if (componentClassName == BoxShapeComponent::k_componentClassName)
		return m_context->getBoxShapePanel();
	if (componentClassName == ModelShapeComponent::k_componentClassName)
		return m_context->getModelShapePanel();

	return nullptr;
}

void GuiPanel_ProjectOutliner::clearComponentPanels()
{
	m_context->getUSBVideoSourcePanel()->setComponent(nullptr);
	m_context->getNetworkVideoSourcePanel()->setComponent(nullptr);
	m_context->getARKitVideoSourcePanel()->setComponent(nullptr);
	m_context->getClientTextureSourcePanel()->setComponent(nullptr);
	m_context->getSpoutTextureSourcePanel()->setComponent(nullptr);
	m_context->getCEFTextureSourcePanel()->setComponent(nullptr);
	m_context->getMarkerPanel()->setComponent(nullptr);
	m_context->getVRTrackingVolumePanel()->setComponent(nullptr);
	m_context->getMarkerTrackingVolumePanel()->setComponent(nullptr);
	m_context->getTrackingMountPanel()->setComponent(nullptr);
	m_context->getStagePanel()->setComponent(nullptr);
	m_context->getCameraPanel()->setComponent(nullptr);
	m_context->getCompositorPanel()->setComponent(nullptr);
	m_context->getScenePanel()->setComponent(nullptr);
	m_context->getLightEnvironmentPanel()->setComponent(nullptr);
	m_context->getSpotLightPanel()->setComponent(nullptr);
	m_context->getPixelGridPanel()->setComponent(nullptr);
	m_context->getAnchorPanel()->setComponent(nullptr);
	m_context->getQuadStencilPanel()->setComponent(nullptr);
	m_context->getBoxStencilPanel()->setComponent(nullptr);
	m_context->getModelStencilPanel()->setComponent(nullptr);
	m_context->getQuadShapePanel()->setComponent(nullptr);
	m_context->getBoxShapePanel()->setComponent(nullptr);
	m_context->getModelShapePanel()->setComponent(nullptr);
}

void GuiPanel_ProjectOutliner::drawComponentPanelForNode(ProjectOutlinerNodePtr node)
{
	if (!node || node->componentId == INVALID_MIKAN_ID)
		return;

	// Video sources use their compact form, matching the old Sources panel
	if (node->componentClassName == USBVideoSourceComponent::k_componentClassName)
	{
		m_context->getUSBVideoSourcePanel()->drawCompactGui();
		return;
	}
	if (node->componentClassName == NetworkVideoSourceComponent::k_componentClassName)
	{
		m_context->getNetworkVideoSourcePanel()->drawCompactGui();
		return;
	}
	if (node->componentClassName == ARKitVideoSourceComponent::k_componentClassName)
	{
		m_context->getARKitVideoSourcePanel()->drawCompactGui();
		return;
	}

	if (GuiPanel_MikanComponent* panel= getPanelForComponentClass(node->componentClassName))
	{
		panel->onGui();
	}
}

bool GuiPanel_ProjectOutliner::isActiveHighlightNode(ProjectOutlinerNodePtr node) const
{
	SceneObjectSystemPtr sceneSystem= m_sceneSystem.lock();
	if (!sceneSystem)
		return false;

	if (node->kind == eOutlinerNodeKind::scene)
	{
		return sceneSystem->getCurrentSceneId() == node->componentId;
	}

	if (node->kind == eOutlinerNodeKind::compositor)
	{
		SceneComponentPtr currentScene= sceneSystem->getCurrentScene();
		return currentScene
			   && currentScene->getSceneComponentDefinition()->getDisplayCompositorId() == node->componentId;
	}

	return false;
}

// -- Object system delegate handlers ----
void GuiPanel_ProjectOutliner::onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr)
{
	markTreeDirty();
}

void GuiPanel_ProjectOutliner::onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr)
{
	markTreeDirty();
}

void GuiPanel_ProjectOutliner::onSystemConfigChanged(CommonConfigPtr configPtr,
													 const ConfigPropertyChangeSet& changedPropertySet)
{
	// Only changes that move a node, rename it, or rewire an ownership edge
	// reshape the tree
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId)
		|| changedPropertySet.hasPropertyName(TransformComponentDefinition::k_parentTransformIdPropertyId)
		|| changedPropertySet.hasPropertyName(StageComponentDefinition::k_trackingVolumeIdPropertyId)
		|| changedPropertySet.hasPropertyName(CameraDefinition::k_ownerStageIdPropertyId)
		|| changedPropertySet.hasPropertyName(CameraDefinition::k_lightEnvironmentIdPropertyId)
		|| changedPropertySet.hasPropertyName(CompositorDefinition::k_ownerScenePropertyId)
		|| changedPropertySet.hasPropertyName(CompositorDefinition::k_cameraIdPropertyId)
		|| changedPropertySet.hasPropertyName(DMXFixtureComponentDefinition::k_ownerStageIdPropertyId)
		|| changedPropertySet.hasPropertyName(VRTrackingVolumeDefinition::k_trackingMountIdsPropertyId))
	{
		markTreeDirty();
	}
}

void GuiPanel_ProjectOutliner::onSelectionChanged()
{
	// A viewport pick selects the matching tree row; a deselect leaves the tree
	// selection alone
	EditorObjectSystemPtr editorSystem= m_editorSystem.lock();
	SelectionComponentPtr selection= editorSystem ? editorSystem->getSelectedSceneActor() : nullptr;
	if (!selection)
		return;

	ProjectOutlinerNodePtr node= m_model.findNodeBySelection(selection);
	if (!node || node->componentId == m_selectedComponentId)
		return;

	setSelectedNode(node, false);

	m_scrollOpenPathIds.clear();
	for (ProjectOutlinerNodePtr pathNode= node->parent.lock(); pathNode; pathNode= pathNode->parent.lock())
	{
		if (pathNode->componentId != INVALID_MIKAN_ID)
			m_scrollOpenPathIds.insert(pathNode->componentId);
	}
	m_bScrollToSelection= true;
}

void GuiPanel_ProjectOutliner::subscribeToSystems()
{
	ProjectManagerPtr projectManager= m_projectManager.lock();
	if (!projectManager)
		return;

	// Subscribe to every system: the property-name filter above keeps systems
	// with no tree presence from triggering rebuilds
	for (MikanObjectSystemPtr system : projectManager->getSystems())
	{
		system->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectOutliner::onObjectInitialized);
		system->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectOutliner::onObjectDisposed);
		if (CommonConfigPtr definition= system->getDefinition())
		{
			definition->OnPropertyChanged+= MakeDelegate(this, &GuiPanel_ProjectOutliner::onSystemConfigChanged);
		}

		m_subscribedSystems.push_back(system);
	}
}

void GuiPanel_ProjectOutliner::unsubscribeFromSystems()
{
	for (MikanObjectSystemWeakPtr systemWeakPtr : m_subscribedSystems)
	{
		MikanObjectSystemPtr system= systemWeakPtr.lock();
		if (!system)
			continue;

		system->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectOutliner::onObjectInitialized);
		system->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectOutliner::onObjectDisposed);
		if (CommonConfigPtr definition= system->getDefinition())
		{
			definition->OnPropertyChanged-= MakeDelegate(this, &GuiPanel_ProjectOutliner::onSystemConfigChanged);
		}
	}
	m_subscribedSystems.clear();
}
