#pragma once

#include "glm/ext/matrix_float4x4.hpp"

#include "openvr.h"

#include <string>
#include <vector>

class SteamVRDeviceProperties
{
public:
	struct RenderComponentInfo
	{
		std::string componentName;
		std::string renderModelName;
		bool isRenderable;
	};

	SteamVRDeviceProperties(vr::TrackedDeviceIndex_t deviceIndex);

	vr::TrackedDeviceIndex_t getSteamVRDeviceIndex() const { return m_deviceIndex; }
	vr::ETrackedDeviceClass getSteamVRDeviceClass() const { return m_deviceClass; }
	const std::string& getDevicePath() const { return m_devicePath; }
	const std::string& getTrackingSystem() const { return m_trackingSystem; }
	const std::string& getTrackingRole() const { return m_trackingRole; }
	const std::string& getModelLabel() const { return m_modelLabel; }
	const std::string& getModelNumber() const { return m_modelNumber; }
	const std::string& getManufacturerName() const { return m_manufacturerName; }
	const std::string& getSerialNumber() const { return m_serialNumber; }
	const std::string& getResourcesPath() const { return m_resourcesPath; }
	const std::string& getReadyIconPath() const { return m_readyIconPath; }
	const std::string& getRenderModelName() const { return m_renderModelName; }
	const std::vector<RenderComponentInfo>& getRenderComponentNames() const { return m_renderComponentNames; }

	void updateProperties();
	void updateResourcesPath();
	bool updateReadyIconPath();
	bool updateRenderModelComponents();

	std::string fetchStringDeviceProperty(
		vr::ETrackedDeviceProperty property,
		const std::string& default_string);
	std::string fetchStringSettingsProperty(
		const char* sectionName,
		const char* sectionKey,
		const std::string& default_string);

private:
	vr::TrackedDeviceIndex_t m_deviceIndex= 0;
	vr::ETrackedDeviceClass m_deviceClass= vr::TrackedDeviceClass_Invalid;
	std::string m_devicePath;
	std::string m_trackingSystem;
	std::string m_trackingRole;
	std::string m_modelLabel;
	std::string m_modelNumber;
	std::string m_manufacturerName;
	std::string m_serialNumber;
	std::string m_resourcesPath;
	std::string m_readyIconPath;
	std::string m_renderModelName;
	std::vector<RenderComponentInfo> m_renderComponentNames;
};