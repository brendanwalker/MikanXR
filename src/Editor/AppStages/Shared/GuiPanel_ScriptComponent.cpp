#include "Shared/GuiPanel_ScriptComponent.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "ScriptComponent.h"

#include "imgui.h"

bool GuiPanel_ScriptComponent::init() { return initTypedPropertyInterface<ScriptComponent>(); }

void GuiPanel_ScriptComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		ScriptDefinition::k_scriptPathPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			ScriptComponentPtr component= getScriptComponent();
			if (!component)
				return false;

			ScriptDefinitionPtr definition= component->getScriptDefinition();
			if (definition->hasScriptPath())
			{
				const std::string scriptPath= definition->getScriptPath().generic_string();

				if (MkGui::drawFilePathProperty(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(ScriptComponent::k_selectScriptFunctionId),
						locText("componentPanel.script"), scriptPath))
				{
					addDeferredGuiEvent([component]() { component->selectScript(); });
				}
				if (!component->isScriptLoaded())
				{
					ImGui::TextUnformatted(locText("componentPanel.scriptNotLoaded"));
				}

				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(ScriptComponent::k_editScriptFunctionId), ICON_FK_PENCIL,
						locText("componentPanel.editScript")))
				{
					addDeferredGuiEvent([component]() { component->editScript(); });
				}
				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(ScriptComponent::k_reloadScriptFunctionId), ICON_FK_REFRESH,
						locText("componentPanel.reloadScript")))
				{
					addDeferredGuiEvent([component]() { component->reloadScript(); });
				}
			}
			else
			{
				MkGui::drawStaticTextProperty(m_defaultGuiStyle, locText("componentPanel.script"),
											  locText("componentPanel.noScript"));

				if (MkGui::drawGlyphButtonWithLabel(
						component->makePropertyUIIdentifier(ScriptComponent::k_selectScriptFunctionId),
						ICON_FK_FOLDER_OPEN, locText("componentPanel.selectScript")))
				{
					addDeferredGuiEvent([component]() { component->selectScript(); });
				}
			}

			return true;
		});
}

void GuiPanel_ScriptComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	ScriptComponentPtr component= getScriptComponent();
	if (!component)
		return;

	// Script triggers as buttons
	std::vector<std::string> triggerNames;
	component->getTriggerNames(triggerNames);
	if (!triggerNames.empty())
	{
		ImGui::TextUnformatted(locText("componentPanel.scriptTriggers"));
		for (const std::string& triggerName : triggerNames)
		{
			if (ImGui::Button(triggerName.c_str()))
			{
				addDeferredGuiEvent([component, triggerName]() { component->invokeTrigger(triggerName); });
			}
		}
	}
}

ScriptComponentPtr GuiPanel_ScriptComponent::getScriptComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<ScriptComponent>(component);
	}
	return nullptr;
}
