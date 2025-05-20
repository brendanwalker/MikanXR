#pragma once

//-- includes -----
#include <memory>
#include <chrono>

//-- typedefs -----
class DeviceView;
typedef std::shared_ptr<DeviceView> DeviceViewPtr;

//-- definitions -----
/// Abstract Base Class for device managers for controllers, trackers, hmds.
class DeviceManager
{
public:
    DeviceManager();
    virtual ~DeviceManager();

    virtual bool startup(class IMkWindow *ownerWindow);
	virtual void update(float deltaTime);
    virtual void shutdown();

    virtual int getMaxDevices() const = 0;

    /**
    Returns an upcast device view ptr. Useful for generic functions that are
    simple wrappers around the device functions:
    open(), getIsOpen(), update(), close(), getIsReadyToPoll()
    For anything that requires knowledge of the device, use the class-specific
    Server<Type>ViewPtr get<Type>ViewPtr(int device_id) functions instead.
    */
    DeviceViewPtr getDeviceViewPtr(int device_id);

protected:
    /** This method tries make the list of open devices in m_devices match
    the list of connected devices in the device enumerator.
    No device objects are created or destroyed.
    Pointers are just shuffled around and devices opened and closed.
    */
    void updateConnectedDeviceViews();

    virtual class DeviceEnumerator* allocateDeviceEnumerator() = 0;
    virtual void freeDeviceEnumerator(class DeviceEnumerator*) = 0;
    virtual DeviceView* allocateDeviceView(int device_id) = 0;

    int findFirstClosedDeviceId();
    int findOpenDeviceId(const class DeviceEnumerator* enumerator);

	class IMkWindow* m_ownerWindow = nullptr;
    DeviceViewPtr* m_deviceViews;
};
