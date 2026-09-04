#include "AppStage.h"
#include "GuiPanel_MikanComponent.h"
#include "MikanComponent.h"
#include "MkGuiStyleManager.h"

const std::string GuiPanel_MikanComponent::k_defaultComponentStyleName= "default_component_panel";

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

void GuiPanel_MikanComponent::onConstruct() {}

void GuiPanel_MikanComponent::onGui()
{
	MikanComponentPtr component= m_component.lock();
	if (!component)
		return;

	// Auto-render all component properties
	m_entityAccessor->onGui();
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
