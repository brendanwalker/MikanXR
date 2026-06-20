#include "AppStage.h"
#include "AssetReferencePropertyMetaData.h"
#include "GuiPanel_MikanComponent.h"
#include "ComponentScriptContext.h"
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

ProjectManagerPtr GuiPanel_MikanComponent::getOwnerProject() const
{
	return getOwnerAppStage()->getProjectManager();
}

MikanComponentPtr GuiPanel_MikanComponent::getComponent() const
{
	return m_component.lock();
}

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
						"Script",
						scriptPath))
				{
					addDeferredGuiEvent([component]()
										{ component->selectComponentScript(); });
				}

				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "", "");
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_editScriptFunctionId),
						"edit_component"))
				{
					addDeferredGuiEvent([component]()
										{ component->editComponentScript(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_reloadScriptFunctionId),
						"reload_component"))
				{
					addDeferredGuiEvent([component]()
										{ component->reloadComponentScript(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_removeScriptFunctionId),
						"delete_component"))
				{
					addDeferredGuiEvent([component]()
										{ component->removeComponentScript(); });
				}
			}
			else
			{
				MkGui::drawStaticTextProperty(m_defaultGuiStyle, "Script", "<No Script>");
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_addNewScriptFunctionId),
						"add_component"))
				{
					addDeferredGuiEvent([component]()
										{ component->addNewComponentScript(); });
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(
						m_defaultGuiStyle,
						component->makePropertyUIIdentifier(MikanComponent::k_selectScriptFunctionId),
						"select_component"))
				{
					addDeferredGuiEvent([component]()
										{ component->selectComponentScript(); });
				}
			}

			return true;
		});
}

void GuiPanel_MikanComponent::onGui()
{
	// Auto-render all component properties
	m_entityAccessor->onGui();

	// Render script triggers as buttons
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		ComponentScriptContextPtr scriptContext= component->getScriptContext();
		if (scriptContext)
		{
			const std::vector<std::string>& triggers= scriptContext->getScriptTriggers();
			if (!triggers.empty())
			{
				ImGui::Separator();
				ImGui::Text("Script Triggers");
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

void GuiPanel_MikanComponent::processDeferredGuiEvents()
{
	m_entityAccessor->processDeferredGuiEvents();
}

void GuiPanel_MikanComponent::dispose()
{
	m_entityAccessor->OnEntityPropertyChanged-= MakeDelegate(
		this,
		&GuiPanel_MikanComponent::onComponentPropertyChanged);
	m_entityAccessor->dispose();

	m_component.reset();
}
