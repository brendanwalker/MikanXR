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

class MarkerDefinition : public MikanComponentDefinition
{
public:
	MarkerDefinition();
	MarkerDefinition(MikanMarkerID markerId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(const Serialization::PolymorphicObjectPtr& initParams) override;

	inline MikanMarkerID getMarkerId() const { return getComponentId(); }

	static const std::string k_arucoIdPropertyId;
	inline int getArucoId() const { return m_arucoId; }
	void setArucoId(int arucoId);

	static const std::string k_lengthMMPropertyId;
	inline float getLengthMM() const { return m_lengthMM; }
	void setLengthMM(float lengthMM);

private:
	int m_arucoId;
	float m_lengthMM;
};

class MarkerComponent : public MikanComponent
{
public:
	MarkerComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "MarkerComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	MarkerObjectSystemPtr getOwnerMarkerSystem() const;
	inline MarkerDefinitionPtr getMarkerDefinition() const
	{
		return std::static_pointer_cast<MarkerDefinition>(m_definition);
	}

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteMarkerFunctionId;
	static const std::string k_printMarkerFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void deleteMarker();
	void printMarker();
};
