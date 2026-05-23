#include "AppStage.h"
#include "AssetReferencePropertyMetaData.h"
#include "GuiPanel_MikanComponent.h"
#include "ComponentScriptContext.h"
#include "MikanComponent.h"
#include "MkGuiStyleManager.h"

#include "imgui.h"

#include "tinyfiledialogs.h"

const std::string GuiPanel_MikanComponent::k_defaultComponentStyleName = "default_component_panel";
const std::string GuiPanel_MikanComponent::k_scriptPathStyleName = "script_path";

GuiPanel_MikanComponent::GuiPanel_MikanComponent(AppStage* ownerAppStage)
	: m_component()
	, m_entityAccessor(std::make_shared<GuiPanel_EntityAccessor>(ownerAppStage))

{
	auto* styleManager= ownerAppStage->getOwnerWindow()->getMkGuiStyleManager();

	m_defaultGuiStyle = styleManager->getStyle(k_defaultComponentStyleName);
	m_scriptPathGuiStyle = styleManager->getStyle(k_scriptPathStyleName);
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
	MikanComponentPtr oldComponent = m_component.lock();

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

		m_component = component;
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
			MikanComponentPtr component = getComponent();
			if (!component) return false;

			if (component->hasValidComponentScript())
			{
				const auto* assetMeta = desc->getMetaDataOfType<AssetReferenceFactoryMetaData>();
				MikanComponentDefinitionPtr componentDef = component->getDefinition();
				const std::string scriptPath = componentDef->getComponentScriptPath().generic_string();

				if (MkGui::drawFilePathProperty(m_scriptPathGuiStyle, "mikanComponentScriptPath", "Script", scriptPath))
				{
					addDeferredGuiEvent([component]() {
						component->addNewComponentScript();
					});
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_scriptPathGuiStyle, "reloadScript", "reload_script"))
				{
					addDeferredGuiEvent([component]() {
						component->reloadComponentScript();
					});
				}
				ImGui::SameLine();
				if (MkGui::drawImageButton(m_scriptPathGuiStyle, "deleteScript", "delete_script"))
				{
					addDeferredGuiEvent([component]() {
						component->removeComponentScript();
					});
				}
			}
			else
			{
				if (ImGui::Button("Add Script"))
				{
					addDeferredGuiEvent([component]() {
						component->addNewComponentScript();
					});
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
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		ComponentScriptContextPtr scriptContext = component->getScriptContext();
		if (scriptContext)
		{
			const std::vector<std::string>& triggers = scriptContext->getScriptTriggers();
			if (!triggers.empty())
			{
				ImGui::Separator();
				ImGui::Text("Script Triggers");
				for (const std::string& triggerName : triggers)
				{
					if (ImGui::Button(triggerName.c_str()))
					{
						addDeferredGuiEvent([scriptContext, triggerName]() {
							scriptContext->invokeScriptTrigger(triggerName);
						});
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
	m_entityAccessor->OnEntityPropertyChanged -= MakeDelegate(
		this,
		&GuiPanel_MikanComponent::onComponentPropertyChanged);
	m_entityAccessor->dispose();

	m_component.reset();
}
