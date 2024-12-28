#pragma once

// -- includes -----
#include "DeviceInterface.h"
#include "RendererFwd.h"

#include <string>
#include <vector>

#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"

#include <stdint.h>

// -- definitions -----
struct CommonVRDeviceState : CommonDeviceState
{
    glm::quat TrackingSpaceOrientation; 
    glm::vec3 TrackingSpacePosition; // Meters
   
    inline CommonVRDeviceState()
    {
        clear();
    }

    inline void clear()
    {
        CommonDeviceState::clear();

        TrackingSpaceOrientation= glm::quat();
        TrackingSpacePosition= glm::vec3();
    }
};

/// Interface class for VR system events. Implemented by VRDeviceManager
class IVRSystemEventListener
{
public:
	virtual ~IVRSystemEventListener() {};

	virtual void onActiveDeviceListChanged() = 0;
	virtual void onDevicePropertyChanged(int deviceId) = 0;
    virtual void onDevicePosesChanged(int64_t newFrameId) = 0;
};

/// Abstract class for VRDevice interface. Implemented by VRDevices
class IVRDeviceInterface : public IDeviceInterface
{
public:
	enum eDriverType
	{
		INVALID,
		SteamVR,

		SUPPORTED_DRIVER_TYPE_COUNT,
	};

	// Returns the driver type being used by this camera
	virtual eDriverType getDriverType() const = 0;

    // -- Mutators
    // Fetch device properties from the VR system
    virtual void updateProperties() = 0;
	// Fetch device pose from the VR system
	virtual void updatePose() = 0;
	// bind to a scene for rendering
	virtual void bindToScene(GlScenePtr scene) = 0;
	// Remove from currently bound scene
	virtual void removeFromBoundScene() = 0;

    // -- Getters
    // Returns the full path to the device
    virtual std::string getDevicePath() const  = 0;
	// Returns the serial number string of the device
	virtual std::string getSerialNumber() const = 0;
	// Returns the assigned "tracking role" the device
	virtual std::string getTrackerRole() const = 0;
	// Pose accessors
	virtual bool getIsPoseValid() const = 0;
	virtual glm::quat getOrientation() const = 0;
	virtual glm::vec3 getPosition() const = 0;
	virtual bool getComponentPoseByName(const std::string& componentName, glm::mat4& outPose) const = 0;
	virtual void getComponentNames(std::vector<std::string> &outComponentName) const = 0;

    // Fetch the latest tracker state from the device
    virtual const CommonVRDeviceState * getVRTrackerState() = 0;
};
