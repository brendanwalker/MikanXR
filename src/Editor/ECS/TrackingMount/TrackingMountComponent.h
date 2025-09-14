#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <memory>
#include <string>

class TrackingMountDefinition : 
	public MikanComponentDefinition
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

private:
	MikanTrackingMountID m_trackingMountId;
	std::string m_devicePath;
	std::string m_socketName;
};

class TrackingMountComponent : public MikanComponent
{
public:
	TrackingMountComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	TrackingMountObjectSystemPtr getOwnerTrackingMountSystem() const;
	inline TrackingMountDefinitionPtr getTrackingMountDefinition() const
	{ return std::static_pointer_cast<TrackingMountDefinition>(m_definition); }

	void deleteTrackingMount();

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ MikanComponent::getRmlFunctionDescriptors(outDescriptors); }
};
