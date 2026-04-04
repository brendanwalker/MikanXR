#include "GuiPanel_MikanComponent.h"
#include "ComponentScriptContext.h"
#include "MikanComponent.h"

#include "imgui.h"

GuiPanel_MikanComponent::GuiPanel_MikanComponent()
	: m_component()
	, m_entityAccessor(std::make_shared<GuiPanel_EntityAccessor>())
{
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

void GuiPanel_MikanComponent::render(float deltaSeconds)
{
	// Auto-render all component properties
	m_entityAccessor->render(deltaSeconds);

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
						addUpdateCallback([scriptContext, triggerName]() {
							scriptContext->invokeScriptTrigger(triggerName);
						});
					}
				}
			}
		}
	}
}

void GuiPanel_MikanComponent::update(float deltaSeconds)
{
	m_entityAccessor->update(deltaSeconds);
}

void GuiPanel_MikanComponent::dispose()
{
	m_entityAccessor->OnEntityPropertyChanged -= MakeDelegate(
		this,
		&GuiPanel_MikanComponent::onComponentPropertyChanged);
	m_entityAccessor->dispose();

	m_component.reset();
}

void GuiPanel_MikanComponent::addUpdateCallback(std::function<void()> callback)
{
	m_entityAccessor->addUpdateCallback(callback);
}
