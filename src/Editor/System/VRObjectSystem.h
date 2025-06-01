#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IVRDeviceManager.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

using VRDeviceMap = std::map<MikanVRDeviceID, VRDeviceComponentWeakPtr>;

class VRObjectSystemConfig : public CommonConfig
{
public:
	VRObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	static const std::string k_AssignedStagePropertyId;
	MikanStageID getAssignedStageId() const { return m_assignedStageId; }
	void setAssignedStageId(MikanStageID stageId);

	static const std::string k_VRDeviceListPropertyId;
	std::vector<VRDeviceDefinitionPtr> vrDeviceList;

	VRDeviceDefinitionPtr getVRDeviceConfig(MikanVRDeviceID vrDeviceId) const;
	VRDeviceDefinitionPtr getVRDeviceConfigByPath(const std::string& vrDevicePath) const;
	MikanVRDeviceID addNewVRDevice(
		const std::string& vrDevicePath,
		const MikanTransform& xform);
	bool removeVRDevice(MikanVRDeviceID vrDeviceId);
	void removeAllVRDevice();

protected:
	MikanVRDeviceID m_nextVRDeviceId= 0;
	MikanStageID m_assignedStageId= 0;
	bool m_bDebugRenderVRs = true;
};

class VRObjectSystem : public MikanObjectSystem, public IVRDeviceManagerListener
{
public:
	static VRObjectSystemPtr getSystem() { return s_VRObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	const ProjectConfig* getProjectConfig() const;
	VRObjectSystemConfigConstPtr getVRSystemConfigConst() const;
	VRObjectSystemConfigPtr getVRSystemConfig();

	const VRDeviceMap& getVRDeviceMap() const { return m_vrDeviceComponents; }
	VRDeviceComponentPtr getVRDeviceById(MikanVRDeviceID vrDeviceId) const;
	VRDeviceComponentPtr getSpatialVRByPath(const std::string& VRDevicePath) const;

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
	void onTrackerRuntimeTypeChaged();

	// VRSystem Config Events
	void onVRSystemConfigMarkedDirty(
		CommonConfigPtr configPtr,
		const class ConfigPropertyChangeSet& changedPropertySet);
	void onAssignedStageChaged();

	// IVRDeviceManagerListener
	virtual void onActiveDeviceListChanged() override;
	virtual void onDevicePropertyChanged(int deviceId) override;
	virtual void onDevicePosesChanged(int64_t newFrameId) override;

private:
	ProjectConfigWeakPtr m_projectConfigWeakPtr;
	eTrackingRuntime m_currentTrackingRuntimeType = eTrackingRuntime::INVALID;
	class IVRDeviceModule* m_vrDeviceModule= nullptr;
	IVRDeviceManagerPtr m_vrDeviceManager= nullptr;
	VRDeviceMap m_vrDeviceComponents;

	static VRObjectSystemWeakPtr s_VRObjectSystem;
};
