#include "GuiPanel_ProjectStages.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "Transform.h"
#include "MikanCoreTypes.h"
#include "IconsForkAwesome.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
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

	m_defaultGuiStyle = getGuiStyleManager()->getStyle("default_component_panel");

	auto pm = ownerAppStage->getProjectManager();
	m_stageDataSource = std::make_unique<GuiDataSource_ComboBox>(pm,
		std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
			{ StageObjectSystem::k_objectSystemClassName, StageComponent::k_componentClassName }
		});

	m_cameraDataSource = std::make_unique<GuiDataSource_ComboBox>(pm,
		std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
			{ CameraObjectSystem::k_objectSystemClassName, CameraComponent::k_componentClassName }
		});
	m_cameraDataSource->setFilter([this](MikanComponentPtr comp) -> bool {
		auto camera = std::static_pointer_cast<CameraComponent>(comp);
		return camera->getCameraDefinition()->getOwnerStageId() == m_selectedStageId;
	});

	// Set initial state based on current scene
	auto sceneSystem = ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	SceneComponentConstPtr currentScene = sceneSystem->getCurrentScene();
	if (currentScene)
	{
		MikanStageID parentStageId = currentScene->getParentStage()->getStageId();
		setSelectedStageId(parentStageId);
	}

	// Auto-select first camera for the selected stage
	m_cameraDataSource->refreshEntries();
	if (m_cameraDataSource->getEntryCount() > 0)
	{
		if (MikanComponentPtr first = m_cameraDataSource->getEntryAtIndex(0))
			setSelectedCameraId((MikanCameraID)first->getComponentId());
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

void GuiPanel_ProjectStages::onGui()
{
	StageObjectSystemPtr stageSystem = getStageSystem();
	CameraObjectSystemPtr cameraSystem = getCameraSystem();
	if (!stageSystem || !cameraSystem)
		return;

	// Stages combo
	m_stageDataSource->refreshEntries();

	if (m_selectedStageId != INVALID_MIKAN_ID &&
		m_stageDataSource->getEntryIndexByComponentId(m_selectedStageId) == -1)
	{
		setSelectedStageId(INVALID_MIKAN_ID);
	}

	int stageIndex = m_stageDataSource->getEntryIndexByComponentId(m_selectedStageId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectStage", "Stage",
		m_stageDataSource.get(), stageIndex))
	{
		if (stageIndex >= 0)
		{
			if (MikanComponentPtr sel = m_stageDataSource->getEntryAtIndex(stageIndex))
			{
				int newId = sel->getComponentId();
				addDeferredGuiEvent([this, newId]() { setSelectedStageId((MikanStageID)newId); });
			}
		}
	}

	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addStage", "add_component"))
	{
		addDeferredGuiEvent([this]() {
			getStageSystem()->addNewObjectByTypedDefinition();
			});
	}

	if (m_selectedStageId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeStage", "delete_component"))
		{
			addDeferredGuiEvent([this]() {
				getStageSystem()->removeObjectByPrimaryComponentId(m_selectedStageId);
			});
		}

		m_context->getStagePanel()->onGui();
	}

	ImGui::Separator();

	m_cameraDataSource->refreshEntries();

	if (m_selectedCameraId != INVALID_MIKAN_ID &&
		m_cameraDataSource->getEntryIndexByComponentId(m_selectedCameraId) == -1)
	{
		setSelectedCameraId(INVALID_MIKAN_ID);
	}

	int cameraIndex = m_cameraDataSource->getEntryIndexByComponentId(m_selectedCameraId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectCamera", "Camera",
		m_cameraDataSource.get(), cameraIndex))
	{
		if (cameraIndex >= 0)
		{
			if (MikanComponentPtr sel = m_cameraDataSource->getEntryAtIndex(cameraIndex))
			{
				int newId = sel->getComponentId();
				addDeferredGuiEvent([this, newId]() { setSelectedCameraId((MikanCameraID)newId); });
			}
		}
	}

	// Cameras combo (filtered by selected stage)
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addCamera", "add_component") &&
		m_selectedStageId != INVALID_MIKAN_ID)
	{
		addDeferredGuiEvent([this]() {
			int stageId = m_selectedStageId;
			getCameraSystem()->addNewObjectByTypedDefinition([stageId](auto def) {
				def->setRelativeTransform(GlmTransform());
				def->setOwnerStageId(stageId);
				def->setParentTransformId(stageId);

				return true;
			});
		});
	}

	if (m_selectedCameraId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeCamera", "delete_component"))
		{
			addDeferredGuiEvent([this]() {
				getCameraSystem()->removeObjectByPrimaryComponentId(m_selectedCameraId);
			});
		}

		m_context->getCameraPanel()->onGui();
	}
}
