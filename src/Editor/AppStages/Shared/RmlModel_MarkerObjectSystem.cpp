#include "MarkerObjectSystem.h"
#include "Shared/RmlModel_MarkerObjectSystem.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "Shared/RmlDataBinding_List.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlModel_MarkerObjectSystem::RmlModel_MarkerObjectSystem()
	: RmlModel_MikanObjectSystem()
{}

bool RmlModel_MarkerObjectSystem::init(
	Rml::Context* rmlContext,
	MikanObjectSystemPtr objectSystem)
{
	bool bSuccess=
		m_propertyInterface->init<MarkerObjectSystem>(
			rmlContext,
			"MarkerObjectSystem",
			[this](Rml::DataModelConstructor& constructor) -> bool
			{
				constructor.BindEventCallback(
					"select_aruco_dictionary",
					[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
						const int enumValue = ev.GetParameter<int>("value", 0);
						const auto dictionaryType = static_cast<eCharucoDictionaryType>(enumValue);
						MarkerObjectSystemConfigPtr systemConfig = getMarkerObjectSystemConfig();
						if (systemConfig)
						{
							systemConfig->setArucoDictionaryType(dictionaryType);
						}
					});

				constructor.BindEventCallback(
					"select_charuco_dictionary",
					[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
						const int enumValue = ev.GetParameter<int>("value", 0);
						const auto dictionaryType = static_cast<eCharucoDictionaryType>(enumValue);
						MarkerObjectSystemConfigPtr systemConfig = getMarkerObjectSystemConfig();
						if (systemConfig)
						{
							systemConfig->setCharucoDictionaryType(dictionaryType);
						}
					});

				constructor.BindEventCallback(
					"select_charuco_rows",
					[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
						const int rowCount = ev.GetParameter<int>("value", 0);
						MarkerObjectSystemConfigPtr systemConfig = getMarkerObjectSystemConfig();
						if (systemConfig)
						{
							systemConfig->setCharucoRows(rowCount);
						}
					});

				constructor.BindEventCallback(
					"select_charuco_cols",
					[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
						const int colCount = ev.GetParameter<int>("value", 0);
						MarkerObjectSystemConfigPtr systemConfig = getMarkerObjectSystemConfig();
						if (systemConfig)
						{
							systemConfig->setCharucoCols(colCount);
						}
					});

				return true;
			});

	return bSuccess;
}

MarkerObjectSystemPtr RmlModel_MarkerObjectSystem::getMarkerObjectSystem() const
{
	MikanObjectSystemPtr objectSystem = m_objectSystem.lock();
	if (objectSystem)
	{
		return std::static_pointer_cast<MarkerObjectSystem>(objectSystem);
	}

	return nullptr;
}

MarkerObjectSystemConfigPtr RmlModel_MarkerObjectSystem::getMarkerObjectSystemConfig() const
{
	auto markerObjectSystem = getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getMarkerSystemConfig();
	}

	return nullptr;
}
