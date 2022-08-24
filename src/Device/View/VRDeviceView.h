#pragma once

//-- includes -----
#include "DeviceView.h"
#include "VRDeviceInterface.h"

#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"

// -- declarations -----
class VRDeviceView : public DeviceView
{
public:
	VRDeviceView(const int device_id);
	~VRDeviceView();

	static IVRDeviceInterface* allocateVRTrackerInterface(const class DeviceEnumerator* enumerator);

	bool open(const class DeviceEnumerator* enumerator) override;
	void close() override;

	IDeviceInterface* getDevice() const override { return m_device; }
	IVRDeviceInterface* getVRDeviceInterface() const { return static_cast<IVRDeviceInterface*>(getDevice()); }

	// Returns what type of tracker this tracker view represents
	eDeviceType getVRDeviceType() const;

	// Returns what type of driver this video source uses
	IVRDeviceInterface::eDriverType getVRTrackerDriverType() const;

	std::string getDevicePath() const;
	std::string getSerialNumber() const;
	std::string getTrackerRole() const;

	bool getIsPoseValid() const;
	glm::quat getOrientation() const;
	glm::vec3 getPosition() const;

	bool getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const;
	void getComponentNames(std::vector<std::string>& outComponentName) const;

	glm::mat4 getCalibrationPose() const;

protected:
	bool allocateDeviceInterface(const class DeviceEnumerator* enumerator) override;
	void freeDeviceInterface() override;

private:
	IVRDeviceInterface* m_device= nullptr;
};
