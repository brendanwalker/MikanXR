#pragma once

#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "MikanPropertyTypes.h"

#include <map>
#include <string>

using PropertyNotifyDatabasePtr= std::shared_ptr<class PropertyNotifyDatabase>;
using PropertyNotifyDatabaseConstPtr= std::shared_ptr<const class PropertyNotifyDatabase>;

using MikanPropertyDatabaseConstPtr= std::shared_ptr<const class MikanPropertyDatabase>;

class PropertyNotifyDatabase
{
public:
	PropertyNotifyDatabase(MikanPropertyDatabaseConstPtr propertyDatabase);

	bool setPropertyNotifyMode(
		const std::string& systemFilter,
		const std::string& componentFilter,
		const std::string& propertyFilter,
		MikanPropertyNotifyMode notifyMode);

	MikanPropertyNotifyMode getPropertyNotifyMode(int propertyIndex) const;
	MikanPropertyNotifyMode getPropertyNotifyMode(
		const MikanPropertyValue& propertyValue,
		ProjectManagerPtr systemManager) const;

private:
	MikanPropertyDatabaseConstPtr m_propertyDatabase;
	std::map<int, MikanPropertyNotifyMode> m_notifyModes;
};