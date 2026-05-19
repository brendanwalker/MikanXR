#include "RGBSpotLightComponent.h"
#include "Colors.h"
#include "DMXFixtureComponent.h"
#include "IDMXManager.h"
#include "MikanLineRenderer.h"
#include "MikanObject.h"
#include "MikanTextRenderer.h"
#include "MikanLightTypes.h"
#include "MikanVariantTypes.h"
#include "SelectionComponent.h"
#include "TextStyle.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- RGBSpotLightDefinition -----
RGBSpotLightDefinition::RGBSpotLightDefinition()
	: DMXFixtureComponentDefinition()
{
	setDMXChannelCount(3);
}

RGBSpotLightDefinition::RGBSpotLightDefinition(MikanLightID lightId)
	: DMXFixtureComponentDefinition(lightId)
{
	setDMXChannelCount(3);
}

configuru::Config RGBSpotLightDefinition::writeToJSON()
{
	configuru::Config pt = DMXFixtureComponentDefinition::writeToJSON();

	return pt;
}

void RGBSpotLightDefinition::readFromJSON(const configuru::Config& pt)
{
	DMXFixtureComponentDefinition::readFromJSON(pt);
}

bool RGBSpotLightDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	return DMXFixtureComponentDefinition::readFromInitParams(ownerObjectSystem, initParams);
}

// -- RGBSpotLightComponent -----
const std::string RGBSpotLightComponent::k_redPropertyId = "rgb_red";
const std::string RGBSpotLightComponent::k_greenPropertyId = "rgb_green";
const std::string RGBSpotLightComponent::k_bluePropertyId = "rgb_blue";

RGBSpotLightComponent::RGBSpotLightComponent(MikanObjectWeakPtr owner)
	: DMXFixtureComponent(owner)
{
}

void RGBSpotLightComponent::init()
{
	DMXFixtureComponent::init();

	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();
}

void RGBSpotLightComponent::setRed(uint8_t v)
{
	m_red = v;
}

void RGBSpotLightComponent::setGreen(uint8_t v)
{
	m_green = v;
}

void RGBSpotLightComponent::setBlue(uint8_t v)
{
	m_blue = v;
}

void RGBSpotLightComponent::setRGB(uint8_t r, uint8_t g, uint8_t b)
{
	m_red = r;
	m_green = g;
	m_blue = b;
}

void RGBSpotLightComponent::sendDMXData(IDMXManager* manager) const
{
	RGBSpotLightDefinitionPtr def = getRGBSpotLightDefinition();
	if (!def || def->getIsDisabled() || !manager)
		return;

	const uint8_t rgb[3] = { getRed(), getGreen(), getBlue() };
	manager->setChannels(
		def->getDMXUniverse(),
		def->getDMXStartChannel(),
		rgb, 3);
}

// -- IEntityAccessor --
rfk::Struct const* RGBSpotLightComponent::getClientAPIValuesStructType() const
{
	return &MikanRGBSpotLightComponentValues::staticGetArchetype();
}

// -- IPropertyInterface --
void RGBSpotLightComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	DMXFixtureComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		RGBSpotLightComponent::k_redPropertyId, MikanVariantType::INT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		RGBSpotLightComponent::k_greenPropertyId, MikanVariantType::INT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		RGBSpotLightComponent::k_bluePropertyId, MikanVariantType::INT));
}

bool RGBSpotLightComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	RGBSpotLightDefinitionPtr def = getRGBSpotLightDefinition();

	if (propertyName == RGBSpotLightComponent::k_redPropertyId)
	{
		outValue = static_cast<int>(getRed());
		return true;
	}
	else if (propertyName == RGBSpotLightComponent::k_greenPropertyId)
	{
		outValue = static_cast<int>(getGreen());
		return true;
	}
	else if (propertyName == RGBSpotLightComponent::k_bluePropertyId)
	{
		outValue = static_cast<int>(getBlue());
		return true;
	}

	return DMXFixtureComponent::getPropertyValue(propertyName, outValue);
}

bool RGBSpotLightComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	RGBSpotLightDefinitionPtr def = getRGBSpotLightDefinition();

	if (propertyName == RGBSpotLightComponent::k_redPropertyId)
	{
		setRed(static_cast<uint8_t>(inValue.getIntValue()));
		return true;
	}
	else if (propertyName == RGBSpotLightComponent::k_greenPropertyId)
	{
		setGreen(static_cast<uint8_t>(inValue.getIntValue()));
		return true;
	}
	else if (propertyName == RGBSpotLightComponent::k_bluePropertyId)
	{
		setBlue(static_cast<uint8_t>(inValue.getIntValue()));
		return true;
	}

	return DMXFixtureComponent::setPropertyValue(propertyName, inValue);
}

// -- customRender --
void RGBSpotLightComponent::customRender(
	IMkGraphicsContext* graphicsContext,
	MikanCameraPtr viewportCamera) const
{
	RGBSpotLightDefinitionPtr def = getRGBSpotLightDefinition();
	if (!def || def->getIsDisabled())
		return;

	const glm::mat4 xform = getWorldTransform();
	const glm::vec3 position = glm::vec3(xform[3]);

	// Color the icon by actual RGB value; fall back to dim white when dark
	const float r = getRed()   / 255.0f;
	const float g = getGreen() / 255.0f;
	const float b = getBlue()  / 255.0f;
	glm::vec3 iconColor = (r + g + b > 0.01f) ? glm::vec3(r, g, b) : Colors::DarkGray;

	SelectionComponentPtr sel = m_selectionComponent.lock();
	if (sel)
	{
		if (sel->getIsSelected())
			iconColor = Colors::Yellow;
		else if (sel->getIsHovered())
			iconColor = Colors::LightGray;
	}

	drawTransformedAxes(graphicsContext, xform, 0.05f, 0.05f, 0.05f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(graphicsContext, style, position,
		L"Light %d [%d,%d,%d]",
		def->getComponentId(),
		static_cast<int>(getRed()),
		static_cast<int>(getGreen()),
		static_cast<int>(getBlue()));
}

// -- Lua Binding --
void RGBSpotLightComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<RGBSpotLightComponent, DMXFixtureComponent>(
			RGBSpotLightComponent::k_componentClassName.c_str())
		.addProperty("red",
			[](RGBSpotLightComponent* c) -> int {
				return static_cast<int>(c->getRed());
			},
			[](RGBSpotLightComponent* c, int v) {
				c->setRed(static_cast<uint8_t>(v));
			})
		.addProperty("green",
			[](RGBSpotLightComponent* c) -> int {
				return static_cast<int>(c->getGreen());
			},
			[](RGBSpotLightComponent* c, int v) {
				c->setGreen(static_cast<uint8_t>(v));
			})
		.addProperty("blue",
			[](RGBSpotLightComponent* c) -> int {
				return static_cast<int>(c->getBlue());
			},
			[](RGBSpotLightComponent* c, int v) {
				c->setBlue(static_cast<uint8_t>(v));
			})
		.addFunction("setRGB",
			[](RGBSpotLightComponent* c, int r, int g, int b) {
				c->setRGB(
					static_cast<uint8_t>(r),
					static_cast<uint8_t>(g),
					static_cast<uint8_t>(b));
			})
		.endClass();
}
