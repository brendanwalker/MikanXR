#pragma once

#include "MikanMathTypes.h"
#include "TrackingMountComponent.h"
#include "TrackingVolumeComponent.h"
#include "Transform.h"
#include "VRTrackingVolumeComponent.h"

#include <vector>

class VRTrackingVolumeDefinition : public TrackingVolumeDefinition
{
public:
	VRTrackingVolumeDefinition();
	VRTrackingVolumeDefinition(
		eTrackingRuntime trackingRuntime,
		MikanTrackingVolumeID trackingVolumeId,
		const std::string& trackingVolumeName);

	virtual eTrackingVolumeType getTrackingVolumeType() const override;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_trackingRuntimePropertyId;
	inline eTrackingRuntime getTrackingRuntime() const { return m_trackingRuntime; }
	void setTrackingRuntime(eTrackingRuntime runtime);

	static const std::string k_charucoMountIdPropertyId;
	inline MikanTrackingMountID getCharucoTrackingMountId() const { return m_charucoMountId; }
	void setCharucoTrackingMountId(MikanTrackingMountID mountId);

	static const std::string k_charucoMountOffsetPropertyId;
	inline MikanVector3f getCharucoMountOffsetMM() const { return m_charucoMountOffsetMM; }
	void setCharucoMountOffsetMM(const MikanVector3f& offset);

	static const std::string k_utilityMarkerIdPropertyId;
	inline MikanMarkerID getUtilityMarkerId() const { return m_utilityMarkerId; }
	void setUtilityMarkerId(MikanMarkerID markerId);

	static const std::string k_trackingMountIdsPropertyId;
	inline const std::vector<MikanTrackingMountID>& getTrackingMountIDs() const { return m_trackingMountIDs; }
	bool addTrackingMountID(MikanTrackingMountID mountId);
	bool removeTrackingMountID(MikanTrackingMountID mountId);

	static const std::string k_vrDevicePoseOffsetPropertyId;
	MikanMatrix4f getVRDevicePoseOffset() const { return m_vrDevicePoseOffset; }
	void setVRDevicePoseOffset(const MikanMatrix4f& poseOffset);

private:
	eTrackingRuntime m_trackingRuntime = eTrackingRuntime::INVALID;
	MikanTrackingMountID m_charucoMountId;
	MikanVector3f m_charucoMountOffsetMM;
	MikanMarkerID m_utilityMarkerId;
	std::vector<MikanTrackingMountID> m_trackingMountIDs;
	MikanMatrix4f m_vrDevicePoseOffset;
};

class VRTrackingVolumeComponent : public TrackingVolumeComponent
{
public:
	VRTrackingVolumeComponent(MikanObjectWeakPtr owner);

	inline VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<VRTrackingVolumeDefinition>(m_definition); }

	inline static const std::string k_componentClassName = "VRTrackingVolumeComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	glm::mat4 getVRDevicePoseOffset() const;
	void setVRDevicePoseOffset(const glm::mat4& poseOffset);

	// -- IPropertyInterface ----
	static const std::string k_vrDevicePositionOffsetPropertyId;
	static const std::string k_vrDeviceRotationOffsetPropertyId;
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_alignTrackingVolumeFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

protected:
	void alignTrackingVolume();

private:
	GlmTransform m_vrDevicePoseOffset;
};