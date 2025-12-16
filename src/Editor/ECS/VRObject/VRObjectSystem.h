#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IVRDeviceManager.h"
#include "MikanTypeFwd.h"
#include "MikanMathTypes.h"
#include "MikanObjectSystem.h"
#include "MkRendererFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

using VRDeviceMap = std::map<MikanVRDeviceID, VRDeviceComponentWeakPtr>;

class VRObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	VRObjectSystemConfig(const std::string& configName);

	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	static const std::string k_trackerRuntimePropertyId;
	eTrackingRuntime getTrackingRuntimeType() const { return m_trackingRuntime; }
	void setTrackingRuntimeType(eTrackingRuntime runtimeType);

	// TODO: Remove this
	const std::string& getDefaultVRObjectSocketName() const;

	// TODO: Move this to stage
	static const std::string k_vrDevicePoseOffsetPropertyId;
	MikanMatrix4f getVRDevicePoseOffset() const { return m_vrDevicePoseOffset; }
	void setVRDevicePoseOffset(const MikanMatrix4f& poseOffset);

	static const std::string k_vrDeviceListPropertyId;
	std::vector<VRDeviceDefinitionPtr> vrDeviceList;

	VRDeviceDefinitionPtr getVRDeviceConfig(MikanVRDeviceID vrDeviceId) const;
	VRDeviceDefinitionPtr getVRDeviceConfigByPath(const std::string& vrDevicePath) const;
	MikanVRDeviceID addNewVRDevice(
		const std::string& vrDevicePath,
		const struct MikanTransform& xform);
	bool removeVRDevice(MikanVRDeviceID vrDeviceId);
	void removeAllVRDevice();

protected:
	eTrackingRuntime m_trackingRuntime = eTrackingRuntime::INVALID;
	MikanVRDeviceID m_nextVRDeviceId= 0;
	MikanMatrix4f m_vrDevicePoseOffset;
	bool m_bDebugRenderVRs = true;
};

class VRObjectSystem : public MikanObjectSystem, public IVRDeviceManagerListener
{
public:
	VRObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	virtual bool init() override;
	virtual void update(float deltaSeconds) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getVRSystemConfigConst();
	}
	VRObjectSystemConfigConstPtr getVRSystemConfigConst() const;
	VRObjectSystemConfigPtr getVRSystemConfig();

	IVRDeviceManagerPtr getVRDeviceManager() const { return m_vrDeviceManager; }
	const VRDeviceMap& getVRDeviceMap() const { return m_vrDeviceComponents; }
	VRDeviceComponentPtr getVRDeviceById(MikanVRDeviceID vrDeviceId) const;
	VRDeviceComponentPtr getVRDeviceByPath(const std::string& VRDevicePath) const;

protected:
	void createVRDeviceManager(eTrackingRuntime desiredRuntime);
	void disposeVRDeviceManager();

	VRDeviceComponentPtr createVRObject(VRDeviceDefinitionPtr vrDeviceDefinition, IVRDevice* vrDeviceInterface);
	void disposeVRObject(MikanVRDeviceID vrDeviceId);
	void disposeAllVRObjects();

	VRDeviceComponentPtr addNewVRDevice(class IVRDevice* vrDeviceInterfac);
	bool removeVRDevice(MikanVRDeviceID VRDeviceId);

	// Project Config Events
	void onProjectConfigMarkedDirty(
		CommonConfigPtr configPtr,
		const class ConfigPropertyChangeSet& changedPropertySet);

	// VRSystem Config Events
	void onVRSystemConfigMarkedDirty(
		CommonConfigPtr configPtr,
		const class ConfigPropertyChangeSet& changedPropertySet);

	// IVRDeviceManagerListener
	virtual void onActiveDeviceListChanged() override;
	virtual void onDevicePropertyChanged(int deviceId) override;
	virtual void onDevicePosesChanged(int64_t newFrameId) override;

private:
	ProjectConfigWeakPtr m_projectConfigWeakPtr;
	class IVRDeviceModule* m_vrDeviceModule= nullptr;
	IVRDeviceManagerPtr m_vrDeviceManager= nullptr;
	VRDeviceMap m_vrDeviceComponents;
};

// -- Utility Methods
void addAllVRDevicesToMkScene(VRObjectSystemPtr vrObjectSystem, IMkScenePtr mkScenePtr);