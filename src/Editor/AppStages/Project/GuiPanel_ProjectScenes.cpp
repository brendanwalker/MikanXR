#include "GuiPanel_ProjectScenes.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "BoxStencilSystem.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "ShapeComponent.h"
#include "StencilComponent.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "EditorObjectSystem.h"
#include "Transform.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
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
#include "Shared/GuiPanel_ShapeComponent.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"
#include "MikanComponent.h"

#include "imgui.h"

bool GuiPanel_ProjectScenes::init(ProjectGuiPanelContext* context)
{
	m_context= context;
	AppStage_Project* ownerAppStage= context->getOwnerAppStage();

	m_anchorSystem= ownerAppStage->getObjectSystemOfType<AnchorObjectSystem>();
	m_compositorSystem= ownerAppStage->getObjectSystemOfType<CompositorObjectSystem>();
	m_editorSystem= ownerAppStage->getObjectSystemOfType<EditorObjectSystem>();
	m_sceneSystem= ownerAppStage->getObjectSystemOfType<SceneObjectSystem>();
	m_stageSystem= ownerAppStage->getObjectSystemOfType<StageObjectSystem>();
	m_quadStencilSystem= ownerAppStage->getObjectSystemOfType<QuadStencilSystem>();
	m_boxStencilSystem= ownerAppStage->getObjectSystemOfType<BoxStencilSystem>();
	m_modelStencilSystem= ownerAppStage->getObjectSystemOfType<ModelStencilSystem>();
	m_quadShapeSystem= ownerAppStage->getObjectSystemOfType<QuadShapeSystem>();
	m_boxShapeSystem= ownerAppStage->getObjectSystemOfType<BoxShapeSystem>();
	m_modelShapeSystem= ownerAppStage->getObjectSystemOfType<ModelShapeSystem>();

	m_defaultGuiStyle= getGuiStyleManager()->getStyle("default_component_panel");

	auto pm= ownerAppStage->getProjectManager();
	m_sceneDataSource= std::make_unique<GuiDataSource_ComboBox>(
		pm, std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
				{SceneObjectSystem::k_objectSystemClassName, SceneComponent::k_componentClassName}});

	m_compositorDataSource= std::make_unique<GuiDataSource_ComboBox>(
		pm, std::vector<GuiDataSource_ComboBox::SystemComponentPair>{
				{CompositorObjectSystem::k_objectSystemClassName, CompositorComponent::k_componentClassName}});
	m_compositorDataSource->setFilter(
		[this](MikanComponentPtr comp) -> bool
		{
			auto compositor= std::static_pointer_cast<CompositorComponent>(comp);
			return compositor && compositor->getCompositorDefinition()->getOwnerSceneId() == m_selectedSceneId;
		});

	// Listen for anchor changes
	AnchorObjectSystemPtr anchorSystem= m_anchorSystem.lock();
	anchorSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onAnchorSystemConfigChanged);
	anchorSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	anchorSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	// Listen for selection changes
	m_editorSystem.lock()->OnSelectionChanged+= MakeDelegate(this, &GuiPanel_ProjectScenes::onSelectionChanged);

	// Listen for stencil changes
	QuadStencilSystemPtr quadStencilSystem= m_quadStencilSystem.lock();
	quadStencilSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	quadStencilSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	quadStencilSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	BoxStencilSystemPtr boxStencilSystem= m_boxStencilSystem.lock();
	boxStencilSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	boxStencilSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	boxStencilSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	ModelStencilSystemPtr modelStencilSystem= m_modelStencilSystem.lock();
	modelStencilSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
	modelStencilSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	modelStencilSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	// Listen for shape changes
	QuadShapeSystemPtr quadShapeSystem= m_quadShapeSystem.lock();
	quadShapeSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
	quadShapeSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	quadShapeSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	BoxShapeSystemPtr boxShapeSystem= m_boxShapeSystem.lock();
	boxShapeSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
	boxShapeSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	boxShapeSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	ModelShapeSystemPtr modelShapeSystem= m_modelShapeSystem.lock();
	modelShapeSystem->getTypedDefinition()->OnPropertyChanged+=
		MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
	modelShapeSystem->OnNewObjectFinalized+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
	modelShapeSystem->OnObjectDisposed+= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);

	// Build initial state
	rebuildSceneOutliner();

	// Set initial selection to current scene
	SceneComponentConstPtr currentScene= m_sceneSystem.lock()->getCurrentScene();
	if (currentScene)
	{
		setSelectedSceneId(currentScene->getSceneId());
	}

	// Auto-select first compositor for the selected scene
	m_compositorDataSource->refreshEntries();
	if (m_compositorDataSource->getEntryCount() > 0)
	{
		if (MikanComponentPtr first= m_compositorDataSource->getEntryAtIndex(0))
			setSelectedCompositorId((MikanCompositorID)first->getComponentId());
	}

	return true;
}

void GuiPanel_ProjectScenes::dispose()
{
	if (AnchorObjectSystemPtr sys= m_anchorSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onAnchorSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (EditorObjectSystemPtr sys= m_editorSystem.lock())
	{
		sys->OnSelectionChanged-= MakeDelegate(this, &GuiPanel_ProjectScenes::onSelectionChanged);
	}

	if (QuadStencilSystemPtr sys= m_quadStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (BoxStencilSystemPtr sys= m_boxStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (ModelStencilSystemPtr sys= m_modelStencilSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onStencilSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (QuadShapeSystemPtr sys= m_quadShapeSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (BoxShapeSystemPtr sys= m_boxShapeSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
	}

	if (ModelShapeSystemPtr sys= m_modelShapeSystem.lock())
	{
		sys->getTypedDefinition()->OnPropertyChanged-=
			MakeDelegate(this, &GuiPanel_ProjectScenes::onShapeSystemConfigChanged);
		sys->OnNewObjectFinalized-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectInitialized);
		sys->OnObjectDisposed-= MakeDelegate(this, &GuiPanel_ProjectScenes::onObjectDisposed);
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
	m_selectedSceneId= sceneId;

	if (SceneComponentPtr scene= getSelectedSceneComponent())
		m_context->getScenePanel()->setComponent(scene);
	else
		m_context->getScenePanel()->setComponent(nullptr);

	// Reset compositor selection
	m_selectedCompositorId= INVALID_MIKAN_ID;
	m_context->getCompositorPanel()->setComponent(nullptr);

	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::setSelectedCompositorId(MikanCompositorID compositorId)
{
	if ((int)compositorId == m_selectedCompositorId)
		return;

	m_selectedCompositorId= (int)compositorId;

	if (CompositorComponentPtr comp= getSelectedCompositor())
		m_context->getCompositorPanel()->setComponent(comp);
	else
		m_context->getCompositorPanel()->setComponent(nullptr);
}

void GuiPanel_ProjectScenes::setSelectedSceneObject(MikanObjectPtr selectedObject)
{
	m_context->getAnchorPanel()->setComponent(nullptr);
	m_context->getBoxStencilPanel()->setComponent(nullptr);
	m_context->getModelStencilPanel()->setComponent(nullptr);
	m_context->getQuadStencilPanel()->setComponent(nullptr);
	m_context->getBoxShapePanel()->setComponent(nullptr);
	m_context->getModelShapePanel()->setComponent(nullptr);
	m_context->getQuadShapePanel()->setComponent(nullptr);

	if (auto component= selectedObject->getComponentOfType<AnchorComponent>())
	{
		m_context->getAnchorPanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<BoxStencilComponent>())
	{
		m_context->getBoxStencilPanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<ModelStencilComponent>())
	{
		m_context->getModelStencilPanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<QuadStencilComponent>())
	{
		m_context->getQuadStencilPanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<BoxShapeComponent>())
	{
		m_context->getBoxShapePanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<ModelShapeComponent>())
	{
		m_context->getModelShapePanel()->setComponent(component);
	}
	else if (auto component= selectedObject->getComponentOfType<QuadShapeComponent>())
	{
		m_context->getQuadShapePanel()->setComponent(component);
	}
}

void GuiPanel_ProjectScenes::onAnchorSystemConfigChanged(CommonConfigPtr configPtr,
														 const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneOutliner();
	}
}

void GuiPanel_ProjectScenes::onStencilSystemConfigChanged(CommonConfigPtr configPtr,
														  const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_parentTransformIdPropertyId)
		|| changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneOutliner();
	}
}

void GuiPanel_ProjectScenes::onShapeSystemConfigChanged(CommonConfigPtr configPtr,
														const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(TransformComponentDefinition::k_parentTransformIdPropertyId)
		|| changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildSceneOutliner();
	}
}

void GuiPanel_ProjectScenes::onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr)
{
	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr)
{
	rebuildSceneOutliner();
}

void GuiPanel_ProjectScenes::onSelectionChanged() { updateSelection(); }

void GuiPanel_ProjectScenes::rebuildSceneOutliner()
{
	m_sceneOutliner.clear();

	SceneComponentPtr sceneComponent= getSelectedSceneComponent();
	if (sceneComponent)
	{
		addTransformComponent(sceneComponent, 0);
	}

	updateSelection();
}

void GuiPanel_ProjectScenes::updateSelection()
{
	m_selectedSceneObjectListIndex= -1;
	m_selectedTransformId= -1;

	SelectionComponentPtr currentSelection= m_editorSystem.lock()->getSelectedSceneActor();
	for (int i= 0; i < (int)m_sceneOutliner.size(); ++i)
	{
		SelectionComponentPtr test= m_sceneOutliner[i].selectionComponent.lock();
		if (test && test == currentSelection)
		{
			m_selectedTransformId= test->getOwnerObject()->getRootComponent()->getComponentId();
			m_selectedSceneObjectListIndex= i;
			break;
		}
	}

	if (m_selectedTransformId == -1)
	{
		// Use the selected scene itself as the transform to attach to
		m_selectedTransformId= m_selectedSceneId;
	}
}

void GuiPanel_ProjectScenes::addTransformComponent(TransformComponentPtr transformComponentPtr, int depth)
{
	if (!transformComponentPtr || transformComponentPtr->getWasDisposed())
		return;

	MikanObjectPtr ownerObject= transformComponentPtr->getOwnerObject();
	if (ownerObject->getRootComponent() == transformComponentPtr)
	{
		const std::string& name= ownerObject->getRootComponent()->getName();
		SelectionComponentPtr selectionComponent= ownerObject->getComponentOfType<SelectionComponent>();

		SceneOutlinerEntry entry;
		entry.name= name.empty() ? locText("project.noName") : name;
		entry.depth= depth;
		entry.selectionComponent= selectionComponent;
		m_sceneOutliner.push_back(entry);
	}

	for (TransformComponentWeakPtr childWeakPtr : transformComponentPtr->getChildTransformComponents())
	{
		TransformComponentPtr child= childWeakPtr.lock();
		if (!child)
			continue;

		MikanObjectPtr childObject= child->getOwnerObject();
		if (childObject != ownerObject)
		{
			// Only show AnchorComponent, StencilComponent, and ShapeComponent-derived objects in the outliner
			if (!childObject->getComponentOfType<AnchorComponent>()
				&& !childObject->getComponentOfType<StencilComponent>()
				&& !childObject->getComponentOfType<ShapeComponent>())
				continue;

			addTransformComponent(child, depth + 1);
		}
		else
		{
			addTransformComponent(child, depth);
		}
	}
}

void GuiPanel_ProjectScenes::onGui()
{
	SceneObjectSystemPtr sceneSystem= m_sceneSystem.lock();
	if (!sceneSystem)
		return;

	// Scenes combo
	m_sceneDataSource->refreshEntries();

	if (m_selectedSceneId != INVALID_MIKAN_ID && m_sceneDataSource->getEntryIndexByComponentId(m_selectedSceneId) == -1)
	{
		setSelectedSceneId(INVALID_MIKAN_ID);
	}

	int sceneIndex= m_sceneDataSource->getEntryIndexByComponentId(m_selectedSceneId);
	if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectScene", locText("project.scene"),
									m_sceneDataSource.get(), sceneIndex))
	{
		if (sceneIndex >= 0)
		{
			if (MikanComponentPtr sel= m_sceneDataSource->getEntryAtIndex(sceneIndex))
			{
				int newId= sel->getComponentId();
				addDeferredGuiEvent([this, newId]() { setSelectedSceneId(newId); });
			}
		}
	}

	ImGui::SameLine();
	if (MkGui::drawImageButton(m_defaultGuiStyle, "addScene", "add_component"))
	{
		addDeferredGuiEvent(
			[this]()
			{
				StageObjectSystemPtr stageSys= m_stageSystem.lock();
				MikanStageID firstStageId= stageSys ? stageSys->getFirstComponentId() : INVALID_MIKAN_ID;
				if (firstStageId != INVALID_MIKAN_ID)
				{
					m_sceneSystem.lock()->addNewObjectByTypedDefinition(
						[firstStageId](auto def)
						{
							def->setParentTransformId(firstStageId);
							return true;
						});
				}
			});
	}

	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "removeScene", "delete_component"))
		{
			addDeferredGuiEvent([this]()
								{ m_sceneSystem.lock()->removeObjectByPrimaryComponentId(m_selectedSceneId); });
		}

		m_context->getScenePanel()->onGui();
	}

	ImGui::Separator();

	// Compositors combo (filtered by selected scene)
	CompositorObjectSystemPtr compositorSystem= m_compositorSystem.lock();
	if (compositorSystem && m_selectedSceneId != INVALID_MIKAN_ID)
	{
		m_compositorDataSource->refreshEntries();

		if (m_selectedCompositorId != INVALID_MIKAN_ID
			&& m_compositorDataSource->getEntryIndexByComponentId(m_selectedCompositorId) == -1)
		{
			setSelectedCompositorId(INVALID_MIKAN_ID);
		}

		int compositorIndex= m_compositorDataSource->getEntryIndexByComponentId(m_selectedCompositorId);
		if (MkGui::drawComboBoxProperty(m_defaultGuiStyle, "projectCompositor", locText("project.compositor"),
										m_compositorDataSource.get(), compositorIndex))
		{
			if (compositorIndex >= 0)
			{
				if (MikanComponentPtr sel= m_compositorDataSource->getEntryAtIndex(compositorIndex))
				{
					int newId= sel->getComponentId();
					addDeferredGuiEvent([this, newId]() { setSelectedCompositorId((MikanCompositorID)newId); });
				}
			}
		}

		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addCompositor", "add_component"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int sceneId= m_selectedSceneId;
					m_compositorSystem.lock()->addNewObjectByTypedDefinition(
						[sceneId](auto def)
						{
							def->setOwnerSceneId(sceneId);
							return true;
						});
				});
		}

		if (m_selectedCompositorId != INVALID_MIKAN_ID)
		{
			ImGui::SameLine();
			if (MkGui::drawImageButton(m_defaultGuiStyle, "removeCompositor", "delete_component"))
			{
				addDeferredGuiEvent(
					[this]() { m_compositorSystem.lock()->removeObjectByPrimaryComponentId(m_selectedCompositorId); });
			}

			m_context->getCompositorPanel()->onGui();
		}
	}

	ImGui::Separator();

	// Scene outliner
	if (m_selectedSceneId != INVALID_MIKAN_ID)
	{
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addAnchor", "add_anchor"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_anchorSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
							def->setParentTransformId(parentTransformId);
							return true;
						});
				});
		}
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addQuadStencil", "add_quad_stencil"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_quadStencilSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
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
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addBoxStencil", "add_box_stencil"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_boxStencilSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
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
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addModelStencil", "add_model_stencil"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_modelStencilSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
							def->setRelativeTransform(GlmTransform());
							def->setParentTransformId(parentTransformId);
							def->setIsDisabled(false);
							return true;
						});
				});
		}

		if (MkGui::drawImageButton(m_defaultGuiStyle, "addQuadShape", "add_quad_shape"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_quadShapeSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
							def->setQuadWidth(0.25f);
							def->setQuadHeight(0.25f);
							def->setIsDoubleSided(true);
							def->setRelativeTransform(GlmTransform());
							def->setParentTransformId(parentTransformId);
							return true;
						});
				});
		}
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addBoxShape", "add_box_shape"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_boxShapeSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
							def->setBoxXSize(0.25f);
							def->setBoxYSize(0.25f);
							def->setBoxZSize(0.25f);
							def->setRelativeTransform(GlmTransform());
							def->setParentTransformId(parentTransformId);
							return true;
						});
				});
		}
		ImGui::SameLine();
		if (MkGui::drawImageButton(m_defaultGuiStyle, "addModelShape", "add_model_shape"))
		{
			addDeferredGuiEvent(
				[this]()
				{
					int parentTransformId= m_selectedTransformId;
					m_modelShapeSystem.lock()->addNewObjectByTypedDefinition(
						[parentTransformId](auto def)
						{
							def->setRelativeTransform(GlmTransform());
							def->setParentTransformId(parentTransformId);
							return true;
						});
				});
		}

		ImGui::TextUnformatted(locText("project.sceneObjects"));
		if (ImGui::BeginListBox("##SceneOutliner", ImVec2(-1, 120)))
		{
			for (int i= 0; i < (int)m_sceneOutliner.size(); ++i)
			{
				const SceneOutlinerEntry& entry= m_sceneOutliner[i];
				std::string indent(entry.depth * 2, ' ');
				std::string label= indent + entry.name + "##obj" + std::to_string(i);

				bool selected= (m_selectedSceneObjectListIndex == i);
				if (ImGui::Selectable(label.c_str(), selected))
				{
					SelectionComponentPtr selComp= entry.selectionComponent.lock();
					if (selComp)
					{
						addDeferredGuiEvent(
							[this, selComp]()
							{
								m_editorSystem.lock()->setSelection(selComp);
								setSelectedSceneObject(selComp->getOwnerObject());
							});
					}
				}
			}
			ImGui::EndListBox();
		}

		// Remove selected object
		if (m_selectedSceneObjectListIndex >= 0 && m_selectedSceneObjectListIndex < (int)m_sceneOutliner.size())
		{
			if (ImGui::Button(locLabel("project.removeSelected")))
			{
				SelectionComponentPtr selComp=
					m_sceneOutliner[m_selectedSceneObjectListIndex].selectionComponent.lock();
				if (selComp)
				{
					MikanObjectPtr ownerObject= selComp->getOwnerObject();
					MikanObjectSystemPtr ownerSystem= ownerObject->getOwnerSystem();

					addDeferredGuiEvent([ownerSystem, ownerObject]() { ownerSystem->deleteObject(ownerObject); });
				}
			}

			m_context->getAnchorPanel()->onGui();
			m_context->getQuadStencilPanel()->onGui();
			m_context->getBoxStencilPanel()->onGui();
			m_context->getModelStencilPanel()->onGui();
			m_context->getQuadShapePanel()->onGui();
			m_context->getBoxShapePanel()->onGui();
			m_context->getModelShapePanel()->onGui();
		}
	}
}
