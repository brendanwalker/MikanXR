#include "AppStage.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "MkGuiDrawUtils.h"

#include "imgui.h"
#include "AssetReferencePropertyMetaData.h"

GuiPanel_CompositorComponent::GuiPanel_CompositorComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_cameraDataSource(ownerAppStage->getProjectManager(),
						 {{CameraObjectSystem::k_objectSystemClassName, CameraComponent::k_componentClassName}})
{
}

bool GuiPanel_CompositorComponent::init() { return initTypedPropertyInterface<CompositorComponent>(); }

void GuiPanel_CompositorComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		CompositorDefinition::k_cameraIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			CompositorComponentPtr compositorComp= getCompositorComponent();
			if (!compositorComp)
				return false;

			m_cameraDataSource.refreshEntries();

			const MikanCameraID currentCameraId= compositorComp->getCompositorDefinition()->getCameraId();
			int selectedIndex= m_cameraDataSource.getEntryIndexByComponentId(currentCameraId);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					compositorComp->makePropertyUIIdentifier(CompositorDefinition::k_cameraIdPropertyId), "Camera",
					&m_cameraDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr newCamera= m_cameraDataSource.getEntryAtIndex(selectedIndex);
					if (newCamera)
					{
						addDeferredGuiEvent(
							[compositorComp, newCamera]()
							{
								compositorComp->getCompositorDefinition()->setCameraId(
									(MikanCameraID)newCamera->getComponentId());
							});
					}
				}
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		CompositorDefinition::k_compositorGraphPathPropertyId,
		[this](const PropertyDescriptorConstPtr& desc) -> bool
		{
			CompositorComponentPtr compositorComp= getCompositorComponent();
			if (!compositorComp)
				return false;

			if (compositorComp->hasValidCompositorGraph())
			{
				const auto* assetMeta= desc->getMetaDataOfType<AssetReferenceFactoryMetaData>();
				CompositorDefinitionPtr componentDef= compositorComp->getCompositorDefinition();
				const std::string scriptPath= componentDef->getCompositorGraphPath().generic_string();

				if (MkGui::drawFilePathProperty(m_defaultGuiStyle,
												compositorComp->makePropertyUIIdentifier(
													CompositorComponent::k_addNewCompositorGraphFunctionId),
												"Graph", scriptPath))
				{
					addDeferredGuiEvent([compositorComp]() { compositorComp->selectCompositorGraph(); });
				}

				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "", "");
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						compositorComp->makePropertyUIIdentifier(CompositorComponent::k_editCompositorGraphFunctionId),
						"edit_component"))
				{
					addDeferredGuiEvent([compositorComp]() { compositorComp->editCompositorGraph(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_defaultGuiStyle,
										   compositorComp->makePropertyUIIdentifier(
											   CompositorComponent::k_removeCompositorGraphFunctionId),
										   "delete_component"))
				{
					addDeferredGuiEvent([compositorComp]() { compositorComp->removeCompositorGraph(); });
				}
			}
			else
			{
				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "Graph", "<No Graph>");
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_defaultGuiStyle,
										   compositorComp->makePropertyUIIdentifier(
											   CompositorComponent::k_addNewCompositorGraphFunctionId),
										   "add_component"))
				{
					addDeferredGuiEvent([compositorComp]() { compositorComp->addNewCompositorGraph(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_defaultGuiStyle,
										   compositorComp->makePropertyUIIdentifier(
											   CompositorComponent::k_selectCompositorGraphFunctionId),
										   "select_component"))
				{
					addDeferredGuiEvent([compositorComp]() { compositorComp->selectCompositorGraph(); });
				}
			}

			return true;
		});
}

CompositorComponentPtr GuiPanel_CompositorComponent::getCompositorComponent() const
{
	return std::static_pointer_cast<CompositorComponent>(m_component.lock());
}
