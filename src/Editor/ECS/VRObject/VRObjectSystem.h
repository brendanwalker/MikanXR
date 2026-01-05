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

	virtual bool wantsSaveForPropertyChange(const ConfigPropertyChangeSet& changedPropertySet) const override;
	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	static const std::string k_vrDeviceListPropertyId;
	std::vector<VRDeviceDefinitionPtr> vrDeviceList;

	VRDeviceDefinitionPtr getVRDeviceConfig(MikanVRDeviceID vrDeviceId) const;
	VRDeviceDefinitionPtr getVRDeviceConfigByPath(const std::string& vrDevicePath) const;
	MikanVRDeviceID addNewVRDevice(
		eTrackingRuntime trackingRuntime,
		const std::string& vrDevicePath,
		const struct MikanTransform& xform);
	bool removeVRDevice(MikanVRDeviceID vrDeviceId);
	void removeAllVRDevice();

protected:
	static const std::string k_nextVRDeviceIdPropertyId;
	MikanVRDeviceID m_nextVRDeviceId= 0;
};

class VRObjectSystem : public MikanObjectSystem, public IVRDeviceManagerListener
{
public:
	VRObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "VRObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init() override;
	virtual void update(float deltaSeconds) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getVRSystemConfigConst();
	}
	VRObjectSystemConfigConstPtr getVRSystemConfigConst() const;
	VRObjectSystemConfigPtr getVRSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	bool createTrackingRuntime(eTrackingRuntime desiredRuntime);
	const VRDeviceMap& getVRDeviceMap() const { return m_vrDeviceComponents; }
	VRDeviceComponentPtr getVRDeviceById(MikanVRDeviceID vrDeviceId) const;
	VRDeviceComponentPtr getVRDeviceByPath(const std::string& VRDevicePath) const;

	MulticastDelegate<void(eTrackingRuntime runtime)> OnActiveDeviceListChanged;
	MulticastDelegate<void(eTrackingRuntime runtime, MikanVRDeviceID deviceId)> OnDevicePropertyChanged;
	MulticastDelegate<void(eTrackingRuntime runtime, int64_t newFrameId)> OnDevicePosesChanged;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	eTrackingRuntime findTrackingRuntimeForDeviceManager(const IVRDeviceManager* deviceManager) const;
	bool findMikanDeviceIdForDeviceIndex(
		const IVRDeviceManager* deviceManager, 
		const int deviceIndex,
		eTrackingRuntime& outRuntime, 
		MikanVRDeviceID& outMikanDeviceId) const;

	VRDeviceComponentPtr createVRObject(VRDeviceDefinitionPtr vrDeviceDefinition, IVRDevice* vrDeviceInterface);
	void disposeVRObject(MikanVRDeviceID vrDeviceId);
	void disposeAllVRObjects();

	VRDeviceComponentPtr addNewVRDevice(eTrackingRuntime trackingRuntime, class IVRDevice* vrDeviceInterfac);
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
	virtual void onActiveDeviceListChanged(IVRDeviceManager* deviceManager) override;
	virtual void onDevicePropertyChanged(IVRDeviceManager* deviceManager, int deviceId) override;
	virtual void onDevicePosesChanged(IVRDeviceManager* deviceManager, int64_t newFrameId) override;

private:
	using VRTrackingRuntimePtr = std::shared_ptr<class VRTrackingRuntime>;
	std::map<eTrackingRuntime, VRTrackingRuntimePtr> m_trackingRuntimes;
	VRDeviceMap m_vrDeviceComponents;
	ProjectConfigWeakPtr m_projectConfigWeakPtr;
};

// -- Utility Methods
void addAllVRDevicesToMkScene(VRObjectSystemPtr vrObjectSystem, IMkScenePtr mkScenePtr);