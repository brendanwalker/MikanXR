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
	MarkerDefinition(
		MikanMarkerID markerId,
		const std::string& markerName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanMarkerID getMarkerId() const { return m_markerId; }

	static const std::string k_arucoIdPropertyId;
	inline int getArucoId() const { return m_arucoId; }
	void setArucoId(int arucoId);

	static const std::string k_lengthMMPropertyId;
	inline float getLengthMM() const { return m_lengthMM; }
	void setLengthMM(float lengthMM);

private:
	MikanMarkerID m_markerId;
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

	//TODO
	//void extractMarkerInfoForClientAPI(struct MikanMarkerInfo& outMarkerInfo) const;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteMarkerFunctionId;
	static const std::string k_printMarkerFunctionId;
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void deleteMarker();
	void printMarker();
};
