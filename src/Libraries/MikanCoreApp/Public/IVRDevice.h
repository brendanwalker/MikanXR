#pragma once

#include "VRDeviceMath.h"
#include "MkRendererFwd.h"

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
class IVRDeviceAttachment
{
public:
	IVRDeviceAttachment() = default;
	virtual ~IVRDeviceAttachment() {}

	virtual const char* getName() const = 0;
	virtual bool getRelativePose(VRDevicePose& outPose) const = 0;
};

class IVRDeviceMesh : public IVRDeviceAttachment
{
public:
	IVRDeviceMesh() = default;
	virtual ~IVRDeviceMesh() {}

	virtual IMkTriangulatedMeshConstPtr getTriangulatedMesh() const = 0;
	virtual IMkWireframeMeshConstPtr getWireframeMesh() const = 0;
};

/// VRDevice interface
class IVRDevice
{
public:
	IVRDevice() = default;
	virtual ~IVRDevice() {}

    // -- Device Properties
	virtual size_t getDeviceIndex() const = 0;
	virtual eVRDeviceType getDeviceType() const = 0;
	virtual const char* getDevicePath() const = 0;
	virtual bool getDevicePose(VRDevicePose& outPose) const = 0;
	virtual const char* getSerialNumber() const = 0;
	virtual const char* getTrackingSystem() const = 0;
	virtual const char* getTrackerRole() const = 0;
	virtual const char* getModelLabel() const = 0;
	virtual const char* getModelNumber() const = 0;
	virtual const char* getManufacturerName() const = 0;

	// -- Attachment accessors
	virtual size_t getAttachmentCount() const = 0;
	virtual IVRDeviceAttachment* getAttachmentByIndex(size_t meshIndex) const = 0;
	virtual IVRDeviceAttachment* getAttachmentByName(const char* meshName) const = 0;

	// -- Mesh accessors
	virtual size_t getMeshCount() const = 0;
	virtual IVRDeviceMesh* getMeshByIndex(size_t meshIndex) const = 0;
	virtual IVRDeviceMesh* getMeshByName(const char* meshName) const = 0;
};
