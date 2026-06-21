#include "PropertyNotifyDatabase.h"
#include "MikanPropertyDatabase.h"
#include "PropertyDatabaseEnumerator.h"
#include "ProjectManager.h"
#include "MikanObjectSystem.h"
#include "MikanComponent.h"

PropertyNotifyDatabase::PropertyNotifyDatabase(MikanPropertyDatabaseConstPtr propertyDatabase)
	: m_propertyDatabase(propertyDatabase)
{
	if (!propertyDatabase)
		return;

	// Iterate over all properties in the database and initialize notify modes to NONE
	const std::vector<MikanPropertyEntry>& allProperties= propertyDatabase->getAllProperties();
	for (const MikanPropertyEntry& entry : allProperties)
	{
		m_notifyModes[entry.propertyIndex]= MikanPropertyNotifyMode::NONE;
	}
}

bool PropertyNotifyDatabase::setPropertyNotifyMode(const std::string& systemFilter, const std::string& componentFilter,
												   const std::string& propertyFilter,
												   MikanPropertyNotifyMode notifyMode)
{
	if (!m_propertyDatabase)
		return false;

	bool anyMatched= false;

	// Use PropertyDatabaseEnumerator to iterate over all matching properties
	PropertyDatabaseEnumerator enumerator(m_propertyDatabase, systemFilter, componentFilter, propertyFilter);

	while (enumerator.isValid())
	{
		int propertyIndex= enumerator.getCurrentPropertyIndex();
		m_notifyModes[propertyIndex]= notifyMode;
		anyMatched= true;

		enumerator.next();
	}

	return anyMatched;
}

MikanPropertyNotifyMode PropertyNotifyDatabase::getPropertyNotifyMode(int propertyIndex) const
{
	auto it= m_notifyModes.find(propertyIndex);
	if (it != m_notifyModes.end())
	{
		return it->second;
	}

	return MikanPropertyNotifyMode::NONE;
}

MikanPropertyNotifyMode PropertyNotifyDatabase::getPropertyNotifyMode(const MikanPropertyValue& propertyValue,
																	  ProjectManagerPtr systemManager) const
{
	if (!m_propertyDatabase || !systemManager)
		return MikanPropertyNotifyMode::NONE;

	const std::string& systemName= propertyValue.ownerSystem.getValue();
	const std::string& propertyName= propertyValue.fieldName.getValue();
	std::string componentClassName;

	// If componentId is -1, this is a system property (no component class name)
	// Otherwise, we need to get the component to determine its class name
	if (propertyValue.componentId != -1)
	{
		MikanObjectSystemPtr system= systemManager->getSystemByName(systemName);
		if (system)
		{
			MikanComponentPtr component= system->getComponentById(propertyValue.componentId);
			if (component)
			{
				// Get the component's type name
				componentClassName= component->getComponentClassName();
			}
		}
	}

	// Look up the property index using the system name, component class name, and property name
	int propertyIndex= m_propertyDatabase->findPropertyIndex(systemName, componentClassName, propertyName);

	// Return the notify mode for this property index
	return getPropertyNotifyMode(propertyIndex);
}
