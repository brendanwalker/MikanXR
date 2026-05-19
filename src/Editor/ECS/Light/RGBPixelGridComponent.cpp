#include "RGBPixelGridComponent.h"
#include "Colors.h"
#include "IDMXManager.h"
#include "MikanLineRenderer.h"
#include "MikanObject.h"
#include "MikanTextRenderer.h"
#include "MikanLightTypes.h"
#include "MikanVariantTypes.h"
#include "SelectionComponent.h"
#include "TextStyle.h"

#include <algorithm>
#include <cstring>

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

// -- RGBPixelGridDefinition -----
const std::string RGBPixelGridDefinition::k_gridColumnsPropertyId = "grid_columns";
const std::string RGBPixelGridDefinition::k_gridRowsPropertyId = "grid_rows";

RGBPixelGridDefinition::RGBPixelGridDefinition()
	: DMXFixtureComponentDefinition()
{
	resizeGrid(m_columns, m_rows);
}

RGBPixelGridDefinition::RGBPixelGridDefinition(MikanLightID gridId)
	: DMXFixtureComponentDefinition(gridId)
{
	resizeGrid(m_columns, m_rows);
}

configuru::Config RGBPixelGridDefinition::writeToJSON()
{
	configuru::Config pt = DMXFixtureComponentDefinition::writeToJSON();

	pt[k_gridColumnsPropertyId] = m_columns;
	pt[k_gridRowsPropertyId]    = m_rows;

	return pt;
}

void RGBPixelGridDefinition::readFromJSON(const configuru::Config& pt)
{
	DMXFixtureComponentDefinition::readFromJSON(pt);

	const int cols = pt.get_or<int>(k_gridColumnsPropertyId, 8);
	const int rows = pt.get_or<int>(k_gridRowsPropertyId, 8);
	resizeGrid(cols, rows);
}

bool RGBPixelGridDefinition::readFromInitParams(
	MikanObjectSystem* ownerObjectSystem,
	const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!DMXFixtureComponentDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* values = initParams.getTypedPointer<MikanRGBPixelGridComponentValues>();
	if (values)
		resizeGrid(values->grid_columns, values->grid_rows);

	return true;
}

void RGBPixelGridDefinition::resizeGrid(int columns, int rows)
{
	m_columns = std::max(1, columns);
	m_rows    = std::max(1, rows);
	setDMXChannelCount(static_cast<uint16_t>(getTotalChannels()));

	notifyPropertyChanged(ConfigPropertyChangeSet()
		.addPropertyName(k_gridColumnsPropertyId)
		.addPropertyName(k_gridRowsPropertyId));
}

void RGBPixelGridDefinition::setColumns(int columns)
{
	if (m_columns != columns)
		resizeGrid(columns, m_rows);
}

void RGBPixelGridDefinition::setRows(int rows)
{
	if (m_rows != rows)
		resizeGrid(m_columns, rows);
}

// -- RGBPixelGridComponent -----
RGBPixelGridComponent::RGBPixelGridComponent(MikanObjectWeakPtr owner)
	: DMXFixtureComponent(owner)
{
}

void RGBPixelGridComponent::init()
{
	DMXFixtureComponent::init();

	m_selectionComponent = getOwnerObject()->getComponentOfType<SelectionComponent>();

	// Ensure pixel data buffer is sized correctly based on definition.
	resizePixelDataBuffer();
}

void RGBPixelGridComponent::onDefinitionMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	DMXFixtureComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);

	if (changedPropertySet.hasPropertyName(RGBPixelGridDefinition::k_gridColumnsPropertyId) ||
		changedPropertySet.hasPropertyName(RGBPixelGridDefinition::k_gridRowsPropertyId))
	{
		resizePixelDataBuffer();
	}
}

void RGBPixelGridComponent::resizePixelDataBuffer()
{
	RGBPixelGridDefinitionConstPtr def = getRGBPixelGridDefinitionConst();
	const size_t requiredSize = static_cast<size_t>(def->getTotalChannels());

	if (m_pixelData.size() != requiredSize)
		m_pixelData.assign(requiredSize, 0);
}

void RGBPixelGridComponent::setPixel(int col, int row, uint8_t r, uint8_t g, uint8_t b)
{
	RGBPixelGridDefinitionConstPtr def = getRGBPixelGridDefinitionConst();

	if (col < 0 || col >= def->getColumns() || row < 0 || row >= def->getRows())
		return;

	const int idx = (row * def->getColumns() + col) * 3;
	m_pixelData[idx + 0] = r;
	m_pixelData[idx + 1] = g;
	m_pixelData[idx + 2] = b;
}

void RGBPixelGridComponent::setAllPixels(const uint8_t* rgbData, int count)
{
	const int copyCount = std::min(count, static_cast<int>(m_pixelData.size()));
	std::memcpy(m_pixelData.data(), rgbData, copyCount);
}

void RGBPixelGridComponent::fillPixels(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < static_cast<int>(m_pixelData.size()); i += 3)
	{
		m_pixelData[i + 0] = r;
		m_pixelData[i + 1] = g;
		m_pixelData[i + 2] = b;
	}
}

void RGBPixelGridComponent::sendDMXData(IDMXManager* manager) const
{
	RGBPixelGridDefinitionPtr def = getRGBPixelGridDefinition();
	if (!def || def->getIsDisabled() || !manager)
		return;

	const std::vector<uint8_t>& pixelData = getPixelData();
	const int totalChannels = def->getTotalChannels();
	if (totalChannels == 0)
		return;

	const uint16_t startUniverse  = def->getDMXUniverse();
	const uint16_t startChannel   = def->getDMXStartChannel(); // 1-based
	const int      startOffset    = startChannel - 1;          // 0-based absolute offset

	// Split pixel data across E1.31 universes (512 slots per universe).
	int channelsRemaining = totalChannels;
	int pixelDataOffset   = 0;

	while (channelsRemaining > 0)
	{
		const int absoluteOffset = startOffset + pixelDataOffset;
		const uint16_t targetUniverse  = static_cast<uint16_t>(startUniverse + (absoluteOffset / 512));
		const uint16_t slotInUniverse  = static_cast<uint16_t>((absoluteOffset % 512) + 1); // 1-based

		const int slotsAvailableInUniverse = 512 - (slotInUniverse - 1);
		const int chunkSize = std::min(channelsRemaining, slotsAvailableInUniverse);

		manager->setChannels(
			targetUniverse,
			slotInUniverse,
			pixelData.data() + pixelDataOffset,
			static_cast<uint16_t>(chunkSize));

		pixelDataOffset   += chunkSize;
		channelsRemaining -= chunkSize;
	}
}

// -- IEntityAccessor --
rfk::Struct const* RGBPixelGridComponent::getClientAPIValuesStructType() const
{
	return &MikanRGBPixelGridComponentValues::staticGetArchetype();
}

// -- IPropertyInterface --
void RGBPixelGridComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	DMXFixtureComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		RGBPixelGridDefinition::k_gridColumnsPropertyId, MikanVariantType::INT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
		RGBPixelGridDefinition::k_gridRowsPropertyId, MikanVariantType::INT));
}

bool RGBPixelGridComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	RGBPixelGridDefinitionPtr def = getRGBPixelGridDefinition();

	if (propertyName == RGBPixelGridDefinition::k_gridColumnsPropertyId)
	{
		outValue = def->getColumns();
		return true;
	}
	else if (propertyName == RGBPixelGridDefinition::k_gridRowsPropertyId)
	{
		outValue = def->getRows();
		return true;
	}

	return DMXFixtureComponent::getPropertyValue(propertyName, outValue);
}

bool RGBPixelGridComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	RGBPixelGridDefinitionPtr def = getRGBPixelGridDefinition();

	if (propertyName == RGBPixelGridDefinition::k_gridColumnsPropertyId)
	{
		def->setColumns(inValue.getIntValue());
		return true;
	}
	else if (propertyName == RGBPixelGridDefinition::k_gridRowsPropertyId)
	{
		def->setRows(inValue.getIntValue());
		return true;
	}

	return DMXFixtureComponent::setPropertyValue(propertyName, inValue);
}

// -- customRender --
void RGBPixelGridComponent::customRender(
	IMkGraphicsContext* graphicsContext,
	MikanCameraPtr viewportCamera) const
{
	RGBPixelGridDefinitionPtr def = getRGBPixelGridDefinition();
	if (!def || def->getIsDisabled())
		return;

	const glm::mat4 xform = getWorldTransform();
	const glm::vec3 position = glm::vec3(xform[3]);

	glm::vec3 iconColor = Colors::DarkGray;
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
		L"PixelGrid %d [%dx%d]",
		def->getComponentId(),
		def->getColumns(),
		def->getRows());
}

// -- Lua Binding --
void RGBPixelGridComponent::bindLuaFunctions(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.deriveClass<RGBPixelGridComponent, DMXFixtureComponent>(
			RGBPixelGridComponent::k_componentClassName.c_str())
		.addProperty("columns",
			[](RGBPixelGridComponent* c) -> int {
				return c->getRGBPixelGridDefinition()->getColumns();
			},
			[](RGBPixelGridComponent* c, int v) {
				c->getRGBPixelGridDefinition()->setColumns(v);
			})
		.addProperty("rows",
			[](RGBPixelGridComponent* c) -> int {
				return c->getRGBPixelGridDefinition()->getRows();
			},
			[](RGBPixelGridComponent* c, int v) {
				c->getRGBPixelGridDefinition()->setRows(v);
			})
		.addFunction("setPixel",
			[](RGBPixelGridComponent* c, int col, int row, int r, int g, int b) {
				c->setPixel(
					col, row,
					static_cast<uint8_t>(r),
					static_cast<uint8_t>(g),
					static_cast<uint8_t>(b));
			})
		.addFunction("fillPixels",
			[](RGBPixelGridComponent* c, int r, int g, int b) {
				c->fillPixels(
					static_cast<uint8_t>(r),
					static_cast<uint8_t>(g),
					static_cast<uint8_t>(b));
			})
		.endClass();
}
