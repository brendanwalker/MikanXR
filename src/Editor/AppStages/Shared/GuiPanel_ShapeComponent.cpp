#include "AppStage.h"
#include "AssetReferencePropertyMetaData.h"
#include "Shared/GuiPanel_ShapeComponent.h"
#include "MkGuiDrawUtils.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "GuiDataSource_ComboBox.h"
#include "MikanCoreTypes.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"

GuiPanel_ShapeComponent::GuiPanel_ShapeComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_parentTransformDataSource(
		  ownerAppStage->getProjectManager(),
		  {{AnchorObjectSystem::k_objectSystemClassName, AnchorComponent::k_componentClassName},
		   {SceneObjectSystem::k_objectSystemClassName, SceneComponent::k_componentClassName},
		   {QuadShapeSystem::k_objectSystemClassName, QuadShapeComponent::k_componentClassName},
		   {BoxShapeSystem::k_objectSystemClassName, BoxShapeComponent::k_componentClassName},
		   {ModelShapeSystem::k_objectSystemClassName, ModelShapeComponent::k_componentClassName}})
{
}

void GuiPanel_ShapeComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// Parent transform picker
	m_entityAccessor->setPropertyRenderer(
		TransformComponentDefinition::k_parentTransformIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			ShapeComponentPtr shapeComponent= getShapeComponent();
			if (!shapeComponent)
				return false;

			m_parentTransformDataSource.refreshEntries();
			if (m_parentTransformDataSource.getEntryCount() == 0)
				return false;

			const MikanTransformID parentTransformId=
				shapeComponent->getShapeComponentDefinition()->getParentTransformId();
			int selectedIndex=
				m_parentTransformDataSource.getEntryIndexByComponentId(parentTransformId);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					shapeComponent->makePropertyUIIdentifier(TransformComponentDefinition::k_parentTransformIdPropertyId),
					"Parent",
					&m_parentTransformDataSource,
					selectedIndex))
			{
				MikanComponentPtr newParent= m_parentTransformDataSource.getEntryAtIndex(selectedIndex);
				if (newParent)
				{
					addDeferredGuiEvent([shapeComponent, newParent]()
										{ shapeComponent->getShapeComponentDefinition()->setParentTransformId(
											  newParent->getComponentId()); });
				}
			}
			return true;
		});

	// Shape node graph path
	m_entityAccessor->setPropertyRenderer(
		ShapeComponentDefinition::k_shapeGraphPathPropertyId,
		[this](const PropertyDescriptorConstPtr& desc) -> bool
		{
			ShapeComponentPtr shapeComponent= getShapeComponent();
			if (!shapeComponent)
				return false;

			if (shapeComponent->hasValidShapeGraph())
			{
				const auto* assetMeta= desc->getMetaDataOfType<AssetReferenceFactoryMetaData>();
				ShapeComponentDefinitionPtr componentDef= shapeComponent->getShapeComponentDefinition();
				const std::string graphPath= componentDef->getShapeGraphPath().generic_string();

				if (MkGui::drawFilePathProperty(
						m_defaultGuiStyle,
						shapeComponent->makePropertyUIIdentifier(ShapeComponent::k_addNewShapeGraphFunctionId),
						"Graph",
						graphPath))
				{
					addDeferredGuiEvent([shapeComponent]()
										{ shapeComponent->selectShapeGraph(); });
				}

				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "", "");
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						shapeComponent->makePropertyUIIdentifier(ShapeComponent::k_editShapeGraphFunctionId),
						"edit_component"))
				{
					addDeferredGuiEvent([shapeComponent]()
										{ shapeComponent->editShapeGraph(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						shapeComponent->makePropertyUIIdentifier(ShapeComponent::k_removeShapeGraphFunctionId),
						"delete_component"))
				{
					addDeferredGuiEvent([shapeComponent]()
										{ shapeComponent->removeShapeGraph(); });
				}
			}
			else
			{
				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "Graph", "<No Graph>");
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						shapeComponent->makePropertyUIIdentifier(ShapeComponent::k_addNewShapeGraphFunctionId),
						"add_component"))
				{
					addDeferredGuiEvent([shapeComponent]()
										{ shapeComponent->addNewShapeGraph(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						shapeComponent->makePropertyUIIdentifier(ShapeComponent::k_selectShapeGraphFunctionId),
						"select_component"))
				{
					addDeferredGuiEvent([shapeComponent]()
										{ shapeComponent->selectShapeGraph(); });
				}
			}

			return true;
		});
}

ShapeComponentPtr GuiPanel_ShapeComponent::getShapeComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<ShapeComponent>(component);
	}
	return nullptr;
}

bool GuiPanel_QuadShapeComponent::init()
{
	return initTypedPropertyInterface<QuadShapeComponent>();
}

bool GuiPanel_BoxShapeComponent::init()
{
	return initTypedPropertyInterface<BoxShapeComponent>();
}

bool GuiPanel_ModelShapeComponent::init()
{
	return initTypedPropertyInterface<ModelShapeComponent>();
}
