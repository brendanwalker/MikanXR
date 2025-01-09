#pragma once

//-- includes -----
#include "DeviceView.h"
#include "VRDeviceInterface.h"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_double4x4.hpp"

#include <string>

// -- constants -----
enum class eVRDevicePoseSpace : int
{
	INVALID,

	VRTrackingSystem,
	MikanScene,

	COUNT,
};

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
	void getComponentNames(std::vector<std::string>& outComponentName) const;
	bool getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const;
	bool getDefaultComponentPose(glm::mat4& outPose) const;
	VRDevicePoseViewPtr makePoseView(eVRDevicePoseSpace space, const std::string& subComponentName= "") const;

protected:
	bool allocateDeviceInterface(const class DeviceEnumerator* enumerator) override;
	void freeDeviceInterface() override;

private:
	IVRDeviceInterface* m_device= nullptr;
};

class VRDevicePoseView
{
public:
	VRDevicePoseView(
		const VRDeviceView* deviceView, 
		eVRDevicePoseSpace space, 
		const std::string& subComponentName= "");

	inline eVRDevicePoseSpace getPoseSpace() const { return m_poseSpace; }

	const VRDeviceView* getDeviceView() const;
	bool getIsPoseValid() const;
	bool getPose(glm::mat4& outPoseInSpace) const;
	bool getPose(glm::dmat4& outPoseInSpace) const;

private:
	VRDeviceViewConstWeakPtr m_deviceView;
	eVRDevicePoseSpace m_poseSpace;
	std::string m_subComponentName;
};