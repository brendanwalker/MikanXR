#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "DMXFixtureComponent.h"
#include "LightSystemFwd.h"
#include "MikanLightTypes.h"

#include <cstdint>
#include <string>
#include <vector>

// -- RGBPixelGridDefinition -----
class RGBPixelGridDefinition : public DMXFixtureComponentDefinition
{
public:
	RGBPixelGridDefinition();
	RGBPixelGridDefinition(MikanLightID gridId);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;
	virtual bool readFromInitParams(
		MikanObjectSystem* ownerObjectSystem,
		const Serialization::PolymorphicObjectPtr& initParams) override;

	static const std::string k_gridColumnsPropertyId;
	int getColumns() const { return m_columns; }
	void setColumns(int columns);

	static const std::string k_gridRowsPropertyId;
	int getRows() const { return m_rows; }
	void setRows(int rows);

	void resizeGrid(int columns, int rows);

	int getPixelCount() const { return m_columns * m_rows; }
	int getTotalChannels() const { return m_columns * m_rows * 3; }

private:
	int m_columns = 8;
	int m_rows = 8;
};

// -- RGBPixelGridComponent -----
class RGBPixelGridComponent : public DMXFixtureComponent
{
public:
	RGBPixelGridComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void customRender(
		IMkGraphicsContext* graphicsContext,
		MikanCameraPtr viewportCamera) const override;

	inline static const std::string k_componentClassName = "RGBPixelGridComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline RGBPixelGridDefinitionConstPtr getRGBPixelGridDefinitionConst() const
	{
		return std::static_pointer_cast<const RGBPixelGridDefinition>(m_definition);
	}
	inline RGBPixelGridDefinitionPtr getRGBPixelGridDefinition() const
	{
		return std::static_pointer_cast<RGBPixelGridDefinition>(m_definition);
	}

	/// Read-only view of the flat R,G,B pixel data (row-major).
	const std::vector<uint8_t>& getPixelData() const { return m_pixelData; }

	/// Set a single pixel.
	void setPixel(int col, int row, uint8_t r, uint8_t g, uint8_t b);

	/// Set all pixels at once. 
	void setAllPixels(const uint8_t* rgbData, int count);

	/// Set all pixels to the same color.
	void fillPixels(uint8_t r, uint8_t g, uint8_t b);

	/// Write all pixel data across one or more E1.31 universes.
	virtual void sendDMXData(IDMXManager* manager) const override;
	virtual void getDMXData(MikanDMXData& outData) const override;
	virtual void setDMXData(const MikanDMXData& data) override;

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
	virtual void onDefinitionMarkedDirty(
		CommonConfigPtr configPtr, 
		const ConfigPropertyChangeSet& changedPropertySet) override;
	void resizePixelDataBuffer();

protected:
	std::vector<uint8_t> m_pixelData; // flat R,G,B triples, row-major; size = columns * rows * 3
	SelectionComponentWeakPtr m_selectionComponent;
};
