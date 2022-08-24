#pragma once

#include <string>

// -- definitions -----
enum class eDeviceType : int
{
	INVALID,
	HMD,
	VRTracker,
    VRController,
	MonoVideoSource,
    StereoVideoSource,

    COUNT
};
extern const std::string* k_deviceTypeStrings;

struct CommonDeviceState
{       
    eDeviceType DeviceType;
    int PollSequenceNumber;
    
    inline CommonDeviceState()
    {
        clear();
    }
    
    inline void clear()
    {
        DeviceType= eDeviceType::INVALID;
        PollSequenceNumber= 0;
    }
};

/// Interface base class for any device interface. Further defined in specific device abstractions.
class IDeviceInterface
{
public:
   
	virtual ~IDeviceInterface() {};

    // Return true if device path matches
    virtual bool matchesDeviceEnumerator(const class DeviceEnumerator *enumerator) const = 0;
    
    // Opens the HID device for the device at the given enumerator
    virtual bool open(const class DeviceEnumerator *enumerator) = 0;
    
    // Returns true if hidapi opened successfully
    virtual bool getIsOpen() const  = 0;
       
    // Closes the HID device for the device
    virtual void close() = 0;
        
    // Returns what type of device
    virtual eDeviceType getDeviceType() const = 0;    
};