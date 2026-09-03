#include "AppStage.h"
#include "AssetReferencePropertyMetaData.h"
#include "GuiPanel_MikanComponent.h"
#include "ComponentScriptContext.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MikanComponent.h"
#include "MkGuiStyleManager.h"

#include "imgui.h"

#include "tinyfiledialogs.h"

const std::string GuiPanel_MikanComponent::k_defaultComponentStyleName= "default_component_panel";
const std::string GuiPanel_MikanComponent::k_scriptPathStyleName= "script_path";

GuiPanel_MikanComponent::GuiPanel_MikanComponent(AppStage* ownerAppStage)
	: m_component()
	, m_entityAccessor(std::make_shared<GuiPanel_EntityAccessor>(ownerAppStage))

{
	auto* styleManager= ownerAppStage->getOwnerWindow()->getMkGuiStyleManager();

	m_defaultGuiStyle= styleManager->getStyle(k_defaultComponentStyleName);
}

ProjectManagerPtr GuiPanel_MikanComponent::getOwnerProject() const { return getOwnerAppStage()->getProjectManager(); }

MikanComponentPtr GuiPanel_MikanComponent::getComponent() const { return m_component.lock(); }

bool GuiPanel_MikanComponent::setComponent(MikanComponentPtr component)
{
	MikanComponentPtr oldComponent= m_component.lock();

	if (component != oldComponent || m_component.expired())
	{
		if (component)
		{
			m_entityAccessor->setEntityAccessor(component);
		}
		else
		{
			m_entityAccessor->setEntityAccessor(nullptr);
		}

		m_component= component;
		return true;
	}

	return false;
}

void GuiPanel_MikanComponent::onConstruct()
{
	m_entityAccessor->setPropertyRenderer(
		MikanComponentDefinition::k_componentScriptPathPropertyId,
		[this](const PropertyDescriptorConstPtr& desc) -> bool
		{
			MikanComponentPtr component= getComponent();
			if (!component)
				return false;

			if (component->hasValidComponentScript())
			{
				const auto* assetMeta= desc->getMetaDataOfType<AssetReferenceFactoryMetaData>();
				MikanComponentDefinitionPtr componentDef= component->getDefinition();
				const std::string scriptPath= componentDef->getComponentScriptPath().generic_string();

				if (MkGui::drawFilePathProperty(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_addNewScriptFunctionId),
						locText("componentPanel.script"), scriptPath))
				{
					addDeferredGuiEvent([component]() { component->selectComponentScript(); });
				}

				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(MikanComponent::k_editScriptFunctionId), ICON_FK_PENCIL,
						locText("componentPanel.editScript")))
				{
					addDeferredGuiEvent([component]() { component->editComponentScript(); });
				}
				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(MikanComponent::k_reloadScriptFunctionId), ICON_FK_REFRESH,
						locText("componentPanel.reloadScript")))
				{
					addDeferredGuiEvent([component]() { component->reloadComponentScript(); });
				}
				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(MikanComponent::k_removeScriptFunctionId), ICON_FK_TRASH_O,
						locText("componentPanel.deleteScript")))
				{
					addDeferredGuiEvent([component]() { component->removeComponentScript(); });
				}
			}
			else
			{
				MkGui::drawStaticTextProperty(m_defaultGuiStyle, locText("componentPanel.script"),
											  locText("componentPanel.noScript"));

				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(MikanComponent::k_addNewScriptFunctionId), ICON_FK_PLUS,
						locText("componentPanel.addScript")))
				{
					addDeferredGuiEvent([component]() { component->addNewComponentScript(); });
				}
				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(MikanComponent::k_selectScriptFunctionId),
						ICON_FK_FOLDER_OPEN, locText("componentPanel.selectScript")))
				{
					addDeferredGuiEvent([component]() { component->selectComponentScript(); });
				}
			}

			return true;
		});
}

void GuiPanel_MikanComponent::onGui()
{
	MikanComponentPtr component= m_component.lock();
	if (!component)
		return;

	// Auto-render all component properties
	m_entityAccessor->onGui();

	// Render script triggers as buttons
	{
		ComponentScriptContextPtr scriptContext= component->getScriptContext();
		if (scriptContext)
		{
			const std::vector<std::string>& triggers= scriptContext->getScriptTriggers();
			if (!triggers.empty())
			{
				ImGui::Separator();
				ImGui::TextUnformatted(locText("componentPanel.scriptTriggers"));
				for (const std::string& triggerName : triggers)
				{
					if (ImGui::Button(triggerName.c_str()))
					{
						addDeferredGuiEvent([scriptContext, triggerName]()
											{ scriptContext->invokeScriptTrigger(triggerName); });
					}
				}
			}
		}
	}
}

void GuiPanel_MikanComponent::addDeferredGuiEvent(std::function<void()> callback)
{
	m_entityAccessor->addDeferredGuiEvent(callback);
}

void GuiPanel_MikanComponent::processDeferredGuiEvents() { m_entityAccessor->processDeferredGuiEvents(); }

void GuiPanel_MikanComponent::dispose()
{
	m_entityAccessor->OnEntityPropertyChanged-=
		MakeDelegate(this, &GuiPanel_MikanComponent::onComponentPropertyChanged);
	m_entityAccessor->dispose();

	m_component.reset();
}
