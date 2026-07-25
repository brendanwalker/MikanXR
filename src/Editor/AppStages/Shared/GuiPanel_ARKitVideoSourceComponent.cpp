#include "AppStage.h"
#include "Shared/GuiPanel_ARKitVideoSourceComponent.h"
#include "ARKitVideoSourceComponent.h"

bool GuiPanel_ARKitVideoSourceComponent::init() { return initTypedPropertyInterface<ARKitVideoSourceComponent>(); }

ARKitVideoSourceComponentPtr GuiPanel_ARKitVideoSourceComponent::getARKitVideoSourceComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<ARKitVideoSourceComponent>(component);
	}
	return nullptr;
}

void GuiPanel_ARKitVideoSourceComponent::drawCompactGui()
{
	GuiPanel_EntityAccessorPtr entityAccessor= getPropertyInterface();

	static const std::set<std::string> compactProperties= {
		MikanComponentDefinition::k_componentNamePropertyId,
		ARKitVideoSourceDefinition::k_basePortPropertyId,
	};
	static const std::set<std::string> compactFunctions= {
		ARKitVideoSourceComponent::k_showVideoSourceSettingsFunctionId};
	entityAccessor->drawPropertiesGui(compactProperties);
	entityAccessor->drawFunctionsGui(compactFunctions);
}
