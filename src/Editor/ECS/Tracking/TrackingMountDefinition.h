#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"
#include "PropertyInterface.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class TrackingMountDefinition : 
	public MikanComponentDefinition,
	public IPropertyInterface
{
public:
	TrackingMountDefinition();
	TrackingMountDefinition(
		MikanTrackingMountID trackingMountId,
		const std::string& mountName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanTrackingMountID getTrackingMountId() const { return m_trackingMountId; }

	static const std::string k_devicePathPropertyId;
	inline const std::string& getDevicePath() const { return m_devicePath; }
	void setDevicePath(const std::string& devicePath);

	static const std::string k_socketNamePropertyId;
	inline const std::string& getSocketName() const { return m_socketName; }
	void setSocketName(const std::string& socketName);

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

private:
	MikanTrackingMountID m_trackingMountId;
	std::string m_devicePath;
	std::string m_socketName;
};
