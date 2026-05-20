#include "GuiPanel_ProjectStages.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "DMXFixtureComponent.h"
#include "Transform.h"
#include "MikanCoreTypes.h"
#include "IconsForkAwesome.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "RGBPixelGridComponent.h"
#include "RGBPixelGridSystem.h"
#include "RGBSpotLightComponent.h"
#include "RGBSpotLightSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "Shared/GuiPanel_CameraComponent.h"
#include "Shared/GuiPanel_RGBPixelGridComponent.h"
#include "Shared/GuiPanel_RGBSpotLightComponent.h"
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
	m_spotLightSystem = ownerAppStage->getObjectSystemOfType<RGBSpotLightSystem>();
	m_pixelGridSystem = ownerAppStage->getObjectSystemOfType<RGBPixelGridSystem>();

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

RGBSpotLightSystemPtr GuiPanel_ProjectStages::getSpotLightSystem() const
{
	return m_spotLightSystem.lock();
}

RGBPixelGridSystemPtr GuiPanel_ProjectStages::getPixelGridSystem() const
{
	return m_pixelGridSystem.lock();
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

	// Reset light selection when stage changes
	m_selectedLightId = INVALID_MIKAN_ID;
	m_context->getSpotLightPanel()->setComponent(nullptr);
	m_context->getPixelGridPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectStages::setSelectedCameraId(MikanCameraID cameraId)
{
	m_selectedCameraId = (int)cameraId;

	if (CameraComponentPtr cameraComponent = getSelectedCamera())
		m_context->getCameraPanel()->setComponent(cameraComponent);
	else
		m_context->getCameraPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectStages::setSelectedLightId(MikanLightID lightId, bool isGrid)
{
	m_selectedLightId = (int)lightId;
	m_selectedLightIsGrid = isGrid;

	if (isGrid)
	{
		RGBPixelGridSystemPtr gridSystem = getPixelGridSystem();
		RGBPixelGridComponentPtr gridComp =
			gridSystem ? gridSystem->getGridById(lightId) : nullptr;
		m_context->getPixelGridPanel()->setComponent(gridComp);
		m_context->getSpotLightPanel()->setComponent(nullptr);
	}
	else
	{
		RGBSpotLightSystemPtr spotSystem = getSpotLightSystem();
		RGBSpotLightComponentPtr spotComp =
			spotSystem ? spotSystem->getLightById(lightId) : nullptr;
		m_context->getSpotLightPanel()->setComponent(spotComp);
		m_context->getPixelGridPanel()->setComponent(nullptr);
	}
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

	ImGui::Separator();

	// Lights section — add spot light and pixel grid buttons
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addSpotLight", "add_spot_light") &&
		m_selectedStageId != INVALID_MIKAN_ID)
	{
		addDeferredGuiEvent([this]() {
			RGBSpotLightSystemPtr spotSystem = getSpotLightSystem();
			if (spotSystem)
			{
				int stageId = m_selectedStageId;
				RGBSpotLightComponentPtr light = spotSystem->addNewObjectByTypedDefinition(
					[stageId](auto def) {
						def->setOwnerStageId((MikanStageID)stageId);
						def->setParentTransformId(stageId);
						def->setRelativeTransform(GlmTransform());
						return true;
					});
				if (light)
					setSelectedLightId((MikanLightID)light->getComponentId(), false);
			}
		});
	}
	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addPixelGrid", "add_pixel_grid") &&
		m_selectedStageId != INVALID_MIKAN_ID)
	{
		addDeferredGuiEvent([this]() {
			RGBPixelGridSystemPtr gridSystem = getPixelGridSystem();
			if (gridSystem)
			{
				int stageId = m_selectedStageId;
				RGBPixelGridComponentPtr grid = gridSystem->addNewObjectByTypedDefinition(
					[stageId](auto def) {
						def->setOwnerStageId((MikanStageID)stageId);
						def->setParentTransformId(stageId);
						def->setRelativeTransform(GlmTransform());
						return true;
					});
				if (grid)
					setSelectedLightId((MikanLightID)grid->getComponentId(), true);
			}
		});
	}

	// Light outliner — flat list of all lights in the selected stage
	if (ImGui::BeginListBox("##LightOutliner", ImVec2(-1, 120)))
	{
		// Spot lights
		RGBSpotLightSystemPtr spotSystem = getSpotLightSystem();
		if (spotSystem)
		{
			for (const auto& [id, weakPtr] : spotSystem->getComponentMap())
			{
				RGBSpotLightComponentPtr comp = weakPtr.lock();
				if (!comp) continue;

				// Filter to selected stage
				if (comp->getDMXFixtureDefinition()->getOwnerStageId() != m_selectedStageId)
					continue;

				const std::string name = comp->getName().empty()
					? ("Light " + std::to_string(id))
					: comp->getName();
				const std::string label = "[Spot] " + name + "##spot" + std::to_string(id);

				bool selected = (!m_selectedLightIsGrid && m_selectedLightId == (int)id);
				if (ImGui::Selectable(label.c_str(), selected))
				{
					int lightId = (int)id;
					addDeferredGuiEvent([this, lightId]() {
						setSelectedLightId((MikanLightID)lightId, false);
					});
				}
			}
		}

		// Pixel grids
		RGBPixelGridSystemPtr gridSystem = getPixelGridSystem();
		if (gridSystem)
		{
			for (const auto& [id, weakPtr] : gridSystem->getComponentMap())
			{
				RGBPixelGridComponentPtr comp = weakPtr.lock();
				if (!comp) continue;

				// Filter to selected stage
				if (comp->getDMXFixtureDefinition()->getOwnerStageId() != m_selectedStageId)
					continue;

				const std::string name = comp->getName().empty()
					? ("Grid " + std::to_string(id))
					: comp->getName();
				const std::string label = "[Grid] " + name + "##grid" + std::to_string(id);

				bool selected = (m_selectedLightIsGrid && m_selectedLightId == (int)id);
				if (ImGui::Selectable(label.c_str(), selected))
				{
					int gridId = (int)id;
					addDeferredGuiEvent([this, gridId]() {
						setSelectedLightId((MikanLightID)gridId, true);
					});
				}
			}
		}

		ImGui::EndListBox();
	}

	// Remove and component panel for the selected light
	if (m_selectedLightId != INVALID_MIKAN_ID)
	{
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeLight", "delete_component"))
		{
			addDeferredGuiEvent([this]() {
				if (m_selectedLightIsGrid)
				{
					RGBPixelGridSystemPtr sys = getPixelGridSystem();
					if (sys)
						sys->removeObjectByPrimaryComponentId(m_selectedLightId);
				}
				else
				{
					RGBSpotLightSystemPtr sys = getSpotLightSystem();
					if (sys)
						sys->removeObjectByPrimaryComponentId(m_selectedLightId);
				}
				setSelectedLightId(INVALID_MIKAN_ID, false);
			});
		}

		if (m_selectedLightIsGrid)
			m_context->getPixelGridPanel()->onGui();
		else
			m_context->getSpotLightPanel()->onGui();
	}
}
