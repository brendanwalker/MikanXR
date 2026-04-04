#include "GuiPanel_ProjectScenes.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilSystem.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "Transform.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "ModelStencilSystem.h"
#include "Project/AppStage_Project.h"
#include "Project/ProjectGuiPanelContext.h"
#include "QuadStencilSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "SelectionComponent.h"
#include "Shared/GuiPanel_AnchorComponent.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"
#include "MikanComponent.h"
#include "TransformComponent.h"

#include "imgui.h"

bool GuiPanel_ProjectScenes::init(ProjectGuiPanelContext* context)
{
	m_context = context;
	AppStage_Project* ownerAppStage = context->getOwnerAppStage();

	m_anchorSystem = ownerAppStage->getObjectSystemOfType<AnchorObjectSystem>();
	m_compositorSystem = ownerAppStage->getObjectSystemOfType<CompositorObjectSystem>();
	m_editorSystem = ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();
	m_sceneSystem = ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	m_stageSystem = ownerAppStage->getObjectSystemOfType<StageObjectSystem>();
	m_quadStencilSystem = ownerAppStage->getObjectSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem = ownerAppStage->getObjectSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem = ownerAppStage->getObjectSystemOfType<ModelStencilSystem>();

	// Listen for anchor changes
	AnchorObjectSystemPtr anchorSystem = m_anchorSystem.lock();
	anchorSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onAnchorSystemConfigChanged);
	anchorSystem->OnObjectInitialized +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	anchorSystem->OnObjectDisposed +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	// Listen for selection changes
	m_editorSystem.lock()->OnSelectionChanged +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onSelectionChanged);

	// Listen for stencil changes
	QuadStencilSystemPtr quadStencilSystem = m_quadStencilSystem.lock();
	quadStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	quadStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	quadStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	BoxStencilSystemPtr boxStencilSystem = m_boxStencilSystem.lock();
	boxStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	boxStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	boxStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	ModelStencilSystemPtr modelStencilSystem = m_modelStencilSystem.lock();
	modelStencilSystem->getTypedDefinition()->OnPropertyChanged +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	modelStencilSystem->OnObjectInitialized +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	modelStencilSystem->OnObjectDisposed +=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	// Build initial state
	rebuildSceneOutliner();

	// Set initial selection to current scene
	SceneComponentConstPtr currentScene = m_sceneSystem.lock()->getCurrentScene();
	if (currentScene)
	{
		setSelectedSceneId(currentScene->getSceneId());
	}

	return true;
}

void GuiPanel_ProjectScenes::dispose()
{
	if (AnchorObjectSystemPtr sys = m_anchorSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onAnchorSystemConfigChanged);
		sys->OnObjectInitialized -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (EditorObjectSystemPtr sys = m_editorSystem.lock())
	{
		sys->OnSelectionChanged -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onSelectionChanged);
	}

	if (QuadStencilSystemPtr sys = m_quadStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnObjectInitialized -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (BoxStencilSystemPtr sys = m_boxStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnObjectInitialized -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (ModelStencilSystemPtr sys = m_modelStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnObjectInitialized -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed -=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	GuiPanel::dispose();
}

SceneComponentPtr GuiPanel_ProjectScenes::getSelectedSceneComponent() const
{
	if (m_selectedSceneId == INVALID_MIKAN_ID)
		return nullptr;
	return m_sceneSystem.lock()->getSceneById(m_selectedSceneId);
}

CompositorComponentPtr GuiPanel_ProjectScenes::getSelectedCompositor() const
{
	if (m_selectedCompositorId == INVALID_MIKAN_ID)
		return nullptr;
	return m_compositorSystem.lock()->getCompositorById((MikanCompositorID)m_selectedCompositorId);
}

void GuiPanel_ProjectScenes::setSelectedSceneId(int sceneId)
{
	if (sceneId == m_selectedSceneId)
		return;

	m_sceneSystem.lock()->setCurrentSceneById(sceneId);
	m_selectedSceneId = sceneId;

	if (SceneComponentPtr scene = getSelectedSceneComponent())
		m_context->getScenePanel()->setComponent(scene);
	else
		m_context->getScenePanel()->setComponent(nullptr);

	// Reset compositor selection
	m_selectedCompositorId = INVALID_MIKAN_ID;
	m_context->getCompositorPanel()->setComponent(nullptr);

	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::setSelectedCompositorId(MikanCompositorID compositorId)
{
	if ((int)compositorId == m_selectedCompositorId)
		return;

	m_selectedCompositorId = (int)compositorId;

	if (CompositorComponentPtr comp = getSelectedCompositor())
		m_context->getCompositorPanel()->setComponent(comp);
	else
		m_context->getCompositorPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectScenes::onAnchorSystemConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneOutliner();
	}
}

void GuiPanel_ProjectScenes::onStencilSystemConfigChanged(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_parentTransformIdPropertyId) ||
		changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneOutliner();
	}
}

void GuiPanel_ProjectScenes::onObjectInitialized(
	MikanObjectSystemPtr objectSystemPtr,
	MikanObjectPtr objectPtr)
{
	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::onObjectDisposed(
	MikanObjectSystemPtr objectSystemPtr,
	MikanObjectConstPtr objectPtr)
{
	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::onSelectionChanged()
{
	updateSelection();
}

void GuiPanel_ProjectScenes::rebuildSceneOutliner()
{
	m_sceneOutliner.clear();

	SceneComponentPtr sceneComponent = getSelectedSceneComponent();
	if (sceneComponent)
	{
		addTransformComponent(sceneComponent, 0);
	}

	updateSelection();
}

void GuiPanel_ProjectScenes::updateSelection()
{
	m_selectedSceneObjectListIndex = -1;
	m_selectedTransformId = -1;

	SelectionComponentPtr currentSelection = m_editorSystem.lock()->getSelection();
	for (int i = 0; i < (int)m_sceneOutliner.size(); ++i)
	{
		SelectionComponentPtr test = m_sceneOutliner[i].selectionComponent.lock();
		if (test && test == currentSelection)
		{
			m_selectedTransformId = test->getOwnerObject()->getRootComponent()->getComponentId();
			m_selectedSceneObjectListIndex = i;
			break;
		}
	}
}

void GuiPanel_ProjectScenes::addTransformComponent(TransformComponentPtr transformComponentPtr, int depth)
{
	if (!transformComponentPtr || transformComponentPtr->getWasDisposed())
		return;

	MikanObjectPtr ownerObject = transformComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == transformComponentPtr)
	{
		const std::string& name = ownerObject->getRootComponent()->getName();
		SelectionComponentPtr selectionComponent = ownerObject->getComponentOfType<SelectionComponent>();

		SceneOutlinerEntry entry;
		entry.name = name.empty() ? "<No Name>" : name;
		entry.depth = depth;
		entry.selectionComponent = selectionComponent;
		m_sceneOutliner.push_back(entry);
	}

	for (TransformComponentWeakPtr childWeakPtr : transformComponentPtr->getChildTransformComponents())
	{
		TransformComponentPtr child = childWeakPtr.lock();
		if (child)
		{
			int childDepth = depth;
			if (child->getOwnerObject() != ownerObject)
				childDepth++;
			addTransformComponent(child, childDepth);
		}
	}
}

void GuiPanel_ProjectScenes::render(float deltaSeconds)
{
	SceneObjectSystemPtr sceneSystem = m_sceneSystem.lock();
	if (!sceneSystem)
		return;

	// Scenes list
	const auto& sceneMap = sceneSystem->getComponentMap();

	// Validate scene selection
	if (m_selectedSceneId != INVALID_MIKAN_ID &&
		sceneMap.find(m_selectedSceneId) == sceneMap.end())
	{
		setSelectedSceneId(INVALID_MIKAN_ID);
	}

	ImGui::Text("Scenes");
	if (ImGui::BeginListBox("##Scenes", ImVec2(-1, 80)))
	{
		for (const auto& [id, weakPtr] : sceneMap)
		{
			SceneComponentPtr scene = std::static_pointer_cast<SceneComponent>(weakPtr.lock());
			if (!scene)
				continue;

			std::string label = scene->getName().empty()
				? ("Scene " + std::to_string(id))
				: scene->getName();
			label += "##scene" + std::to_string(id);

			bool selected = (m_selectedSceneId == (int)id);
			if (ImGui::Selectable(label.c_str(), selected))
			{
				int newId = (int)id;
				addUpdateCallback([this, newId]() {
					setSelectedSceneId(newId);
				});
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::Button("Add Scene"))
	{
		addUpdateCallback([this]() {
			StageObjectSystemPtr stageSys = m_stageSystem.lock();
			MikanStageID firstStageId = stageSys ? stageSys->getFirstComponentId() : INVALID_MIKAN_ID;
			if (firstStageId != INVALID_MIKAN_ID)
			{
				m_sceneSystem.lock()->addNewObjectByTypedDefinition(
					[firstStageId](auto def) {
						def->setParentStageId(firstStageId);
						return true;
					});
			}
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Scene") && m_selectedSceneId != INVALID_MIKAN_ID)
	{
		addUpdateCallback([this]() {
			m_sceneSystem.lock()->removeObjectByPrimaryComponentId(m_selectedSceneId);
		});
	}

	ImGui::Separator();

	// Compositors list (filtered by selected scene)
	CompositorObjectSystemPtr compositorSystem = m_compositorSystem.lock();
	if (compositorSystem && m_selectedSceneId != INVALID_MIKAN_ID)
	{
		ImGui::Text("Compositors");

		if (ImGui::BeginListBox("##Compositors", ImVec2(-1, 80)))
		{
			for (const auto& [id, weakPtr] : compositorSystem->getComponentMap())
			{
				CompositorComponentPtr comp = std::static_pointer_cast<CompositorComponent>(weakPtr.lock());
				if (!comp)
					continue;
				if (comp->getCompositorDefinition()->getOwnerSceneId() != m_selectedSceneId)
					continue;

				std::string label = comp->getName().empty()
					? ("Compositor " + std::to_string(id))
					: comp->getName();
				label += "##comp" + std::to_string(id);

				bool selected = (m_selectedCompositorId == (int)id);
				if (ImGui::Selectable(label.c_str(), selected))
				{
					int newId = (int)id;
					addUpdateCallback([this, newId]() {
						setSelectedCompositorId((MikanCompositorID)newId);
					});
				}
			}
			ImGui::EndListBox();
		}

		if (ImGui::Button("Add Compositor"))
		{
			addUpdateCallback([this]() {
				int sceneId = m_selectedSceneId;
				m_compositorSystem.lock()->addNewObjectByTypedDefinition(
					[sceneId](auto def) {
						def->setOwnerSceneId(sceneId);
						return true;
					});
			});
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove Compositor") && m_selectedCompositorId != INVALID_MIKAN_ID)
		{
			addUpdateCallback([this]() {
				m_compositorSystem.lock()->removeObjectByPrimaryComponentId(m_selectedCompositorId);
			});
		}
	}

	ImGui::Separator();

	// Scene outliner
	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		ImGui::Text("Scene Objects");
		if (ImGui::BeginListBox("##SceneOutliner", ImVec2(-1, 120)))
		{
			for (int i = 0; i < (int)m_sceneOutliner.size(); ++i)
			{
				const SceneOutlinerEntry& entry = m_sceneOutliner[i];
				std::string indent(entry.depth * 2, ' ');
				std::string label = indent + entry.name + "##obj" + std::to_string(i);

				bool selected = (m_selectedSceneObjectListIndex == i);
				if (ImGui::Selectable(label.c_str(), selected))
				{
					SelectionComponentPtr selComp = entry.selectionComponent.lock();
					if (selComp)
					{
						addUpdateCallback([this, selComp]() {
							m_editorSystem.lock()->setSelection(selComp);
						});
					}
				}
			}
			ImGui::EndListBox();
		}

		// Add/Remove object buttons
		if (ImGui::Button("Add Anchor"))
		{
			addUpdateCallback([this]() {
				int parentTransformId = m_selectedTransformId;
				m_anchorSystem.lock()->addNewObjectByTypedDefinition(
					[parentTransformId](auto def) {
						def->setParentTransformId(parentTransformId);
						return true;
					});
			});
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Quad Stencil"))
		{
			addUpdateCallback([this]() {
				int parentTransformId = m_selectedTransformId;
				m_quadStencilSystem.lock()->addNewObjectByTypedDefinition(
					[parentTransformId](auto def) {
						def->setQuadWidth(0.25f);
						def->setQuadHeight(0.25f);
						def->setIsDoubleSided(true);
						def->setRelativeTransform(GlmTransform());
						def->setParentTransformId(parentTransformId);
						def->setIsDisabled(false);
						return true;
					});
			});
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Box Stencil"))
		{
			addUpdateCallback([this]() {
				int parentTransformId = m_selectedTransformId;
				m_boxStencilSystem.lock()->addNewObjectByTypedDefinition(
					[parentTransformId](auto def) {
						def->setBoxXSize(0.25f);
						def->setBoxYSize(0.25f);
						def->setBoxZSize(0.25f);
						def->setRelativeTransform(GlmTransform());
						def->setParentTransformId(parentTransformId);
						def->setIsDisabled(false);
						return true;
					});
			});
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Model Stencil"))
		{
			addUpdateCallback([this]() {
				int parentTransformId = m_selectedTransformId;
				m_modelStencilSystem.lock()->addNewObjectByTypedDefinition(
					[parentTransformId](auto def) {
						def->setRelativeTransform(GlmTransform());
						def->setParentTransformId(parentTransformId);
						def->setIsDisabled(false);
						return true;
					});
			});
		}

		// Remove selected object
		if (m_selectedSceneObjectListIndex >= 0 &&
			m_selectedSceneObjectListIndex < (int)m_sceneOutliner.size())
		{
			ImGui::SameLine();
			if (ImGui::Button("Remove Selected"))
			{
				SelectionComponentPtr selComp =
					m_sceneOutliner[m_selectedSceneObjectListIndex].selectionComponent.lock();
				if (selComp)
				{
					// Find what system owns this component and remove it
					MikanObjectPtr ownerObject = selComp->getOwnerObject();
					MikanObjectSystemPtr ownerSystem = ownerObject->getOwnerSystem();

					addUpdateCallback([ownerSystem, ownerObject]() {
						ownerSystem->deleteObject(ownerObject);
					});
				}
			}
		}
	}
}
