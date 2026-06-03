#include "AppStage.h"
#include "Shared/GuiPanel_ShapeComponent.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "CEFTextureSourceComponent.h"
#include "CEFTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "ClientTextureSourceSystem.h"
#include "GuiDataSource_ComboBox.h"
#include "MikanCoreTypes.h"
#include "MkGuiDrawUtils.h"
#include "NetworkVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "BoxShapeComponent.h"
#include "BoxShapeSystem.h"
#include "ModelShapeComponent.h"
#include "ModelShapeSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "TextureSourceComponent.h"
#include "TransformComponent.h"
#include "USBVideoSourceComponent.h"
#include "USBVideoSourceSystem.h"

GuiPanel_ShapeComponent::GuiPanel_ShapeComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_parentTransformDataSource(
		ownerAppStage->getProjectManager(),
		{
			{ AnchorObjectSystem::k_objectSystemClassName, AnchorComponent::k_componentClassName },
			{ SceneObjectSystem::k_objectSystemClassName, SceneComponent::k_componentClassName },
			{ QuadShapeSystem::k_objectSystemClassName, QuadShapeComponent::k_componentClassName },
			{ BoxShapeSystem::k_objectSystemClassName, BoxShapeComponent::k_componentClassName },
			{ ModelShapeSystem::k_objectSystemClassName, ModelShapeComponent::k_componentClassName }
		})
	, m_textureSourceDataSource(
		ownerAppStage->getProjectManager(),
		{
			{ ClientTextureSourceSystem::k_objectSystemClassName, ClientTextureSourceComponent::k_componentClassName },
			{ SpoutTextureSourceSystem::k_objectSystemClassName, SpoutTextureSourceComponent::k_componentClassName },
			{ CEFTextureSourceSystem::k_objectSystemClassName, CEFTextureSourceComponent::k_componentClassName },
			{ NetworkVideoSourceSystem::k_objectSystemClassName, NetworkVideoSourceComponent::k_componentClassName },
			{ USBVideoSourceSystem::k_objectSystemClassName, USBVideoSourceComponent::k_componentClassName }
		})
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
			ShapeComponentPtr shapeComponent = getShapeComponent();
			if (!shapeComponent)
				return false;

			m_parentTransformDataSource.refreshEntries();
			if (m_parentTransformDataSource.getEntryCount() == 0)
				return false;

			const MikanTransformID parentTransformId =
				shapeComponent->getShapeComponentDefinition()->getParentTransformId();
			int selectedIndex =
				m_parentTransformDataSource.getEntryIndexByComponentId(parentTransformId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle,
				shapeComponent->makePropertyUIIdentifier(TransformComponentDefinition::k_parentTransformIdPropertyId),
				"Parent",
				&m_parentTransformDataSource,
				selectedIndex))
			{
				MikanComponentPtr newParent = m_parentTransformDataSource.getEntryAtIndex(selectedIndex);
				if (newParent)
				{
					addDeferredGuiEvent([shapeComponent, newParent]() {
						shapeComponent->getShapeComponentDefinition()->setParentTransformId(
							newParent->getComponentId());
					});
				}
			}
			return true;
		});

	// Texture source picker
	m_entityAccessor->setPropertyRenderer(
		ShapeComponentDefinition::k_textureSourceIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			ShapeComponentPtr shapeComponent = getShapeComponent();
			if (!shapeComponent)
				return false;

			m_textureSourceDataSource.refreshEntries();

			const MikanTextureSourceID textureSourceId = shapeComponent->getTextureSourceId();
			int selectedIndex =
				m_textureSourceDataSource.getEntryIndexByComponentId(textureSourceId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle,
				shapeComponent->makePropertyUIIdentifier(ShapeComponentDefinition::k_textureSourceIdPropertyId),
				"Texture Source",
				&m_textureSourceDataSource,
				selectedIndex))
			{
				MikanComponentPtr newSource = m_textureSourceDataSource.getEntryAtIndex(selectedIndex);
				addDeferredGuiEvent([shapeComponent, newSource]() {
					const MikanTextureSourceID newId = newSource ? newSource->getComponentId() : INVALID_MIKAN_ID;
					shapeComponent->setTextureSourceId(newId);
				});
			}
			return true;
		});
}

ShapeComponentPtr GuiPanel_ShapeComponent::getShapeComponent() const
{
	MikanComponentPtr component = m_component.lock();
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
