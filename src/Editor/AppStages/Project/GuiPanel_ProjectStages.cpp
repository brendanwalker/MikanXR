#include "GuiPanel_ProjectStages.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "Transform.h"
#include "MikanCoreTypes.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "Shared/GuiPanel_StageComponent.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"

#include "imgui.h"

bool GuiPanel_ProjectStages::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();
	m_stageSystem = ownerAppStage->getObjectSystemOfType<StageObjectSystem>();
	m_cameraSystem = ownerAppStage->getObjectSystemOfType<CameraObjectSystem>();

	// Set initial state based on current scene
	auto sceneSystem = ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	SceneComponentConstPtr currentScene = sceneSystem->getCurrentScene();
	if (currentScene)
	{
		MikanStageID parentStageId = currentScene->getParentStage()->getStageId();
		setSelectedStageId(parentStageId);
	}

	return true;
}

void GuiPanel_ProjectStages::dispose()
{
	GuiPanel::dispose();
}

StageObjectSystemPtr GuiPanel_ProjectStages::getStageSystem() const
{
	return m_stageSystem.lock();
}

CameraObjectSystemPtr GuiPanel_ProjectStages::getCameraSystem() const
{
	return m_cameraSystem.lock();
}

StageComponentPtr GuiPanel_ProjectStages::getSelectedStage() const
{
	StageObjectSystemPtr sys = getStageSystem();
	if (sys && m_selectedStageId != INVALID_MIKAN_ID)
		return sys->getStageById((MikanStageID)m_selectedStageId);
	return nullptr;
}

CameraComponentPtr GuiPanel_ProjectStages::getSelectedCamera() const
{
	CameraObjectSystemPtr sys = getCameraSystem();
	if (sys && m_selectedCameraId != INVALID_MIKAN_ID)
		return sys->getCameraById((MikanCameraID)m_selectedCameraId);
	return nullptr;
}

void GuiPanel_ProjectStages::setSelectedStageId(MikanStageID stageId)
{
	m_selectedStageId = (int)stageId;

	if (StageComponentPtr stageComponent = getSelectedStage())
		m_context->getStagePanel()->setComponent(stageComponent);
	else
		m_context->getStagePanel()->setComponent(nullptr);

	// Reset camera selection when stage changes
	m_selectedCameraId = INVALID_MIKAN_ID;
	m_context->getCameraPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectStages::setSelectedCameraId(MikanCameraID cameraId)
{
	m_selectedCameraId = (int)cameraId;

	if (CameraComponentPtr cameraComponent = getSelectedCamera())
		m_context->getCameraPanel()->setComponent(cameraComponent);
	else
		m_context->getCameraPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectStages::render(float deltaSeconds)
{
	StageObjectSystemPtr stageSystem = getStageSystem();
	CameraObjectSystemPtr cameraSystem = getCameraSystem();
	if (!stageSystem || !cameraSystem)
		return;

	// Validate stage selection
	const auto& stageMap = stageSystem->getComponentMap();
	if (m_selectedStageId != INVALID_MIKAN_ID &&
		stageMap.find(m_selectedStageId) == stageMap.end())
	{
		setSelectedStageId(INVALID_MIKAN_ID);
	}

	// Stages list
	if (ImGui::BeginListBox("Stages", ImVec2(-1, 100)))
	{
		for (const auto& [id, weakPtr] : stageMap)
		{
			StageComponentPtr stage = std::static_pointer_cast<StageComponent>(weakPtr.lock());
			if (!stage)
				continue;

			std::string label = stage->getName().empty()
				? ("Stage " + std::to_string(id))
				: stage->getName();
			label += "##stage" + std::to_string(id);

			bool selected = (m_selectedStageId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addUpdateCallback([this, newId]() {
					setSelectedStageId((MikanStageID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add Stage"))
	{
		addUpdateCallback([this]() {
			getStageSystem()->addNewObjectByTypedDefinition();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Stage") && m_selectedStageId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			getStageSystem()->removeObjectByPrimaryComponentId(m_selectedStageId);
		});
	}

	ImGui::Separator();

	// Cameras list (filtered by selected stage)
	const auto* cameraConfig = cameraSystem->getTypedDefinition().get();
	if (ImGui::BeginListBox("Cameras", ImVec2(-1, 100)))
	{
		for (const auto& [id, weakPtr] : cameraSystem->getComponentMap())
		{
			CameraComponentPtr camera = std::static_pointer_cast<CameraComponent>(weakPtr.lock());
			if (!camera)
				continue;
			if (camera->getCameraDefinition()->getOwnerStageId() != m_selectedStageId)
				continue;

			std::string label = camera->getName().empty()
				? ("Camera " + std::to_string(id))
				: camera->getName();
			label += "##camera" + std::to_string(id);

			bool selected = (m_selectedCameraId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addUpdateCallback([this, newId]() {
					setSelectedCameraId((MikanCameraID)newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add Camera") && m_selectedStageId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			int stageId = m_selectedStageId;
			getCameraSystem()->addNewObjectByTypedDefinition([stageId](auto def) {
				def->setRelativeTransform(GlmTransform());
				def->setOwnerStageId(stageId);
				return true;
			});
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Camera") && m_selectedCameraId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			getCameraSystem()->removeObjectByPrimaryComponentId(m_selectedCameraId);
		});
	}
}
