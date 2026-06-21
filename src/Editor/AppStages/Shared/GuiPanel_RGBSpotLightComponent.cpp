#include "AppStage.h"
#include "RGBSpotLightComponent.h"
#include "Shared/GuiPanel_RGBSpotLightComponent.h"

#include "imgui.h"

bool GuiPanel_RGBSpotLightComponent::init() { return initTypedPropertyInterface<RGBSpotLightComponent>(); }

void GuiPanel_RGBSpotLightComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(RGBSpotLightComponent::k_redPropertyId,
										  [this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
										  {
											  RGBSpotLightComponentPtr comp= getRGBSpotLightComponent();
											  if (!comp)
												  return false;

											  int red= (int)comp->getRed();
											  if (ImGui::SliderInt("Red", &red, 0, 255))
											  {
												  addDeferredGuiEvent(
													  [this, red]()
													  {
														  if (auto c= getRGBSpotLightComponent())
															  c->setRed((uint8_t)red);
													  });
											  }
											  return true;
										  });

	m_entityAccessor->setPropertyRenderer(RGBSpotLightComponent::k_greenPropertyId,
										  [this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
										  {
											  RGBSpotLightComponentPtr comp= getRGBSpotLightComponent();
											  if (!comp)
												  return false;

											  int green= (int)comp->getGreen();
											  if (ImGui::SliderInt("Green", &green, 0, 255))
											  {
												  addDeferredGuiEvent(
													  [this, green]()
													  {
														  if (auto c= getRGBSpotLightComponent())
															  c->setGreen((uint8_t)green);
													  });
											  }
											  return true;
										  });

	m_entityAccessor->setPropertyRenderer(RGBSpotLightComponent::k_bluePropertyId,
										  [this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
										  {
											  RGBSpotLightComponentPtr comp= getRGBSpotLightComponent();
											  if (!comp)
												  return false;

											  int blue= (int)comp->getBlue();
											  if (ImGui::SliderInt("Blue", &blue, 0, 255))
											  {
												  addDeferredGuiEvent(
													  [this, blue]()
													  {
														  if (auto c= getRGBSpotLightComponent())
															  c->setBlue((uint8_t)blue);
													  });
											  }
											  return true;
										  });
}

RGBSpotLightComponentPtr GuiPanel_RGBSpotLightComponent::getRGBSpotLightComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
		return std::static_pointer_cast<RGBSpotLightComponent>(component);
	return nullptr;
}
