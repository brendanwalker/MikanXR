#pragma once

// -- includes -----
enum class eVRDeviceType : int
{
	INVALID,
	HMD,
	VRTracker,
	VRController,

	COUNT
};

// -- definitions -----
struct VRDevicePosition
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

struct VRDeviceQuat
{
	float w= 1.f;
	float x= 0.f;
	float y= 0.f;
	float z= 0.f;
};

struct VRDevicePose
{
	VRDevicePosition position;
	VRDeviceQuat orientation;
};

/// VRDevice interface
class IVRDevice 
{
public:
	IVRDevice() = default;
	virtual ~IVRDevice() {}

	// Needed?
    // -- Mutators
    // Fetch device properties from the VR system
    //virtual void updateProperties() = 0;
	// Fetch device pose from the VR system
	//virtual void updatePose() = 0;

    // -- Getters
	// Returns what type of device
	virtual eVRDeviceType getDeviceType() const = 0;
    // Returns the full path to the device
    virtual const char* getDevicePath() const  = 0;
	// Returns the serial number string of the device
	virtual const char* getSerialNumber() const = 0;
	// Returns the assigned "tracking role" the device
	virtual const char* getTrackerRole() const = 0;
	// Pose accessors
	virtual bool getDevicePose(VRDevicePose& outPose) const = 0;
	virtual size_t getComponentCount() const = 0;
	virtual const char* getComponentName(size_t componentIndex) const = 0;
	virtual bool getComponentPoseByName(const char* componentName, VRDevicePose& outPose) const = 0;
};
