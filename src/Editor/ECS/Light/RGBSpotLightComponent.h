#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "DMXFixtureComponent.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"

#include <cstdint>
#include <string>

// ============================================================
// RGBSpotLightDefinition
// ============================================================

class RGBSpotLightDefinition : public DMXFixtureComponentDefinition
{
public:
	RGBSpotLightDefinition();
	RGBSpotLightDefinition(MikanLightID lightId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;
	virtual bool readFromInitParams(
		MikanObjectSystem* ownerObjectSystem,
		const Serialization::PolymorphicObjectPtr& initParams) override;
};

// ============================================================
// RGBSpotLightComponent
// ============================================================

class RGBSpotLightComponent : public DMXFixtureComponent
{
public:
	RGBSpotLightComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void customRender(
		IMkGraphicsContext* graphicsContext,
		MikanCameraPtr viewportCamera) const override;

	inline static const std::string k_componentClassName = "RGBSpotLightComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline RGBSpotLightDefinitionPtr getRGBSpotLightDefinition() const
	{
		return std::static_pointer_cast<RGBSpotLightDefinition>(m_definition);
	}

	static const std::string k_redPropertyId;
	uint8_t getRed() const { return m_red; }
	void setRed(uint8_t v);

	static const std::string k_greenPropertyId;
	uint8_t getGreen() const { return m_green; }
	void setGreen(uint8_t v);

	static const std::string k_bluePropertyId;
	uint8_t getBlue() const { return m_blue; }
	void setBlue(uint8_t v);

	void setRGB(uint8_t r, uint8_t g, uint8_t b);

	virtual void sendDMXData(IDMXManager* manager) const override;

	// -- IEntityAccessor --
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface --
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface --
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
	{ DMXFixtureComponent::getFunctionDescriptors(outDescriptors); }

	// -- Lua Binding --
	static void bindLuaFunctions(struct lua_State* L);

protected:
	uint8_t m_red = 0;
	uint8_t m_green = 0;
	uint8_t m_blue = 0;
	SelectionComponentWeakPtr m_selectionComponent;
};
