#include "Shared/GuiPanel_ScriptComponent.h"
#include "AppStage.h"
#include "IconsForkAwesome.h"
#include "IEditorWindow.h"
#include "LocText.h"
#include "MikanVariantTypes.h"
#include "MkGuiDrawUtils.h"
#include "ScriptComponent.h"
#include "TransactionHistory.h"

#include "imgui.h"

#include <string.h>

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

	TransactionHistory* transactionHistory= getOwnerAppStage()->getOwnerWindow()->getTransactionHistory();

	// Script triggers as buttons. A trigger run is bracketed as one gesture so
	// its property writes coalesce; object creates and destroys still seal on
	// their own.
	std::vector<std::string> triggerNames;
	component->getTriggerNames(triggerNames);
	if (!triggerNames.empty())
	{
		ImGui::TextUnformatted(locText("componentPanel.scriptTriggers"));
		for (const std::string& triggerName : triggerNames)
		{
			if (ImGui::Button(triggerName.c_str()))
			{
				addDeferredGuiEvent(
					[component, triggerName, transactionHistory]()
					{
						if (transactionHistory != nullptr)
							transactionHistory->beginGesture("script:" + triggerName);
						component->invokeTrigger(triggerName);
						if (transactionHistory != nullptr)
							transactionHistory->endGesture();
					});
			}
		}
	}

	// Script variables as widgets, labeled by their Lua global name
	std::vector<std::string> variableNames;
	component->getScriptVariableNames(variableNames);
	if (!variableNames.empty())
	{
		ImGui::TextUnformatted(locText("componentPanel.scriptVariables"));
		for (const std::string& variableName : variableNames)
		{
			drawScriptVariable(component, variableName, transactionHistory);
		}
	}
}

void GuiPanel_ScriptComponent::drawScriptVariable(ScriptComponentPtr component, const std::string& variableName,
												  TransactionHistory* transactionHistory)
{
	MikanVariant value;
	if (!component->getScriptVariable(variableName, value))
		return;

	const std::string uiFieldId= component->makePropertyUIIdentifier("var_" + variableName);
	const char* label= variableName.c_str();

	bool bValueChanged= false;
	MikanVariant newValue= value;

	switch (value.value_type)
	{
	case MikanVariantType::BOOL:
	{
		bool v= value.getBoolValue();
		if (MkGui::drawCheckBoxProperty(m_defaultGuiStyle, uiFieldId, label, v))
		{
			newValue= v;
			bValueChanged= true;
		}
	}
	break;
	case MikanVariantType::INT:
	{
		int v= value.getIntValue();
		if (MkGui::drawIntProperty(m_defaultGuiStyle, uiFieldId, label, v))
		{
			newValue= v;
			bValueChanged= true;
		}
	}
	break;
	case MikanVariantType::FLOAT:
	{
		float v= value.getFloatValue();
		if (MkGui::drawFloatProperty(m_defaultGuiStyle, uiFieldId, label, v))
		{
			newValue= v;
			bValueChanged= true;
		}
	}
	break;
	case MikanVariantType::STRING:
	{
		char buf[256];
		strncpy_s(buf, sizeof(buf), value.getUtf8Value(), _TRUNCATE);
		if (MkGui::drawStringProperty(m_defaultGuiStyle, uiFieldId, label, buf, sizeof(buf)))
		{
			newValue= buf;
			bValueChanged= true;
		}
	}
	break;
	case MikanVariantType::VECTOR3F:
	{
		const MikanVector3f& vec= value.getVector3fValue();
		float v[3]= {vec.x, vec.y, vec.z};
		if (MkGui::drawFloat3Property(m_defaultGuiStyle, uiFieldId, label, v))
		{
			newValue= MikanVector3f{v[0], v[1], v[2]};
			bValueChanged= true;
		}
	}
	break;
	default:
		return;
	}

	// Bracket an in-progress widget edit as one transaction gesture, keeping
	// the per-frame value writes flowing for live preview
	if (transactionHistory != nullptr)
	{
		if (ImGui::IsItemActive())
		{
			transactionHistory->beginGesture("script_var:" + variableName);
		}
		else if (ImGui::IsItemDeactivated())
		{
			transactionHistory->endGesture();
		}
	}

	if (bValueChanged)
	{
		addDeferredGuiEvent([component, variableName, newValue]() mutable
							{ component->setScriptVariable(variableName, newValue); });
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
