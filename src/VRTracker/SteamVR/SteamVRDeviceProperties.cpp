#include "MathTypeConversion.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "SteamVRDeviceProperties.h"

SteamVRDeviceProperties::SteamVRDeviceProperties(
	vr::TrackedDeviceIndex_t deviceIndex)
{
	m_deviceIndex = deviceIndex;
}

void SteamVRDeviceProperties::updateProperties()
{
	vr::IVRSystem* vrSystem = vr::VRSystem();
	if (vrSystem == nullptr)
		return;

	m_deviceClass = vrSystem->GetTrackedDeviceClass(m_deviceIndex);
	m_trackingSystem = fetchStringDeviceProperty(vr::Prop_TrackingSystemName_String, "");
	m_modelLabel = fetchStringDeviceProperty(vr::Prop_ModeLabel_String, "");
	m_modelNumber = fetchStringDeviceProperty(vr::Prop_ModelNumber_String, "");
	m_manufacturerName = fetchStringDeviceProperty(vr::Prop_ManufacturerName_String, "");
	m_serialNumber = fetchStringDeviceProperty(vr::Prop_SerialNumber_String, "");

	std::string registeredDeviceType = fetchStringDeviceProperty(vr::Prop_RegisteredDeviceType_String, "");
	if (registeredDeviceType.size() > 0)
	{
		m_devicePath= "/devices/"+ registeredDeviceType;
	}

	// Determine the tracking role of the device
	switch (m_deviceClass)
	{
	case vr::TrackedDeviceClass_HMD:
		m_trackingRole= "Head";
		break;
	case vr::TrackedDeviceClass_Controller:
		{
			vr::ETrackedControllerRole controllerRole= 
				vrSystem->GetControllerRoleForTrackedDeviceIndex(m_deviceIndex);

			if (controllerRole == vr::TrackedControllerRole_LeftHand)
			{
				m_trackingRole = "LeftHand";
			}
			else if (controllerRole == vr::TrackedControllerRole_RightHand)
			{
				m_trackingRole = "RightHand";
			}
			else
			{
				m_trackingRole = "Controller";
			}
		}
		break;
	case vr::TrackedDeviceClass_GenericTracker:
		{
			std::string rawTrackingRole= 
				fetchStringSettingsProperty(vr::k_pch_Trackers_Section, m_devicePath.c_str(), "tracker");
			std::vector<std::string> rawRoleParts= StringUtils::splitString(rawTrackingRole, '_');

			m_trackingRole= rawRoleParts[rawRoleParts.size() - 1];
		}
		break;
	}

	updateResourcesPath();

	if (updateReadyIconPath())
	{
		//UpdateReadyIconImage();
	}

	if (updateRenderModelComponents())
	{
		//UpdateRenderModel();
	}
}

void SteamVRDeviceProperties::updateResourcesPath()
{
	if (m_trackingSystem.size() == 0)
		return;

	std::string resourcesPath = PathUtils::getResourceDirectory();
	if (resourcesPath.size() == 0)
		return;

	if (PathUtils::doesDirectoryExist(resourcesPath))
	{
		m_resourcesPath = resourcesPath;
	}
}

bool SteamVRDeviceProperties::updateReadyIconPath()
{
	std::string newIconPath = "";

	if (m_resourcesPath.size() != 0)
	{
		std::string partialIconPath = fetchStringDeviceProperty(vr::Prop_NamedIconPathDeviceReady_String, "");
		if (partialIconPath.size() != 0)
		{
			std::string resourcesToken = "{{"+ m_trackingSystem +"}}";

			size_t startPos= partialIconPath.find(resourcesToken);
			if (startPos != std::string::npos)
			{
				std::string fullIconPath = partialIconPath.replace(startPos, resourcesToken.size(), m_resourcesPath + "\\icons\\");

				if (PathUtils::doesFileExist(fullIconPath))
				{
					newIconPath = fullIconPath;
				}
			}
		}
	}

	if (m_readyIconPath != newIconPath)
	{
		m_readyIconPath = newIconPath;
		return true;
	}

	return false;
}

bool SteamVRDeviceProperties::updateRenderModelComponents()
{
	std::string newRenderModelName = "";

	if (m_resourcesPath.size() > 0)
	{
		newRenderModelName = fetchStringDeviceProperty(vr::Prop_RenderModelName_String, "");
	}

	if (m_renderModelName == newRenderModelName)
	{
		// Bail if the render model name didn't change
		return true;
	}

	// Update the render model name
	m_renderModelName = newRenderModelName;

	// Flush the list of render component names
	m_renderComponentNames.clear();

	// Bail if we have no render model name
	if (m_renderModelName.size() == 0)
	{
		return false;
	}

	// Bail if the render model interface isn't available
	vr::IVRRenderModels* renderModelInterface= vr::VRRenderModels();
	if (renderModelInterface == nullptr)
	{
		return false;
	}

	// Rebuild the renderComponent list
	uint32_t componentCount = renderModelInterface->GetComponentCount(m_renderModelName.c_str());
	if (componentCount > 0) 
	{
		for (uint32_t componentIndex = 0; componentIndex < componentCount; ++componentIndex)
		{
			// Get the length of the component name
			const uint32_t componentNameLen = 
				renderModelInterface->GetComponentName(
					m_renderModelName.c_str(), 
					componentIndex, 
					nullptr, 
					0);
			if (componentNameLen != 0)
			{
				// Allocate a buffer for the component name
				char* szComponentName = new char[componentNameLen + 1];

				// Fetch the component name
				if (renderModelInterface->GetComponentName(
					m_renderModelName.c_str(),
					componentIndex,
					szComponentName,
					componentNameLen) != 0)
				{
					// Get the length of the component's render model name
					// NOTE: Some components are dynamic and don't have meshes
					const uint32_t componentRenderModelNameLen =
						renderModelInterface->GetComponentRenderModelName(
							m_renderModelName.c_str(),
							szComponentName,
							nullptr,
							0);

					if (componentRenderModelNameLen != 0)
					{
						// Allocate a buffer for the render model name
						char* szRenderModelName = new char[componentRenderModelNameLen + 1];

						// Fetch the render model name
						if (renderModelInterface->GetComponentRenderModelName(
							m_renderModelName.c_str(),
							szComponentName,
							szRenderModelName,
							componentRenderModelNameLen) != 0)
						{
							// Add to the list of components
							RenderComponentInfo componentInfo;
							componentInfo.componentName= szComponentName;
							componentInfo.renderModelName= szRenderModelName;
							componentInfo.isRenderable= true;

							m_renderComponentNames.push_back(componentInfo);
						}

						delete[] szRenderModelName;
					}
					else
					{
						// Add to the list of components
						RenderComponentInfo componentInfo;
						componentInfo.componentName = szComponentName;
						componentInfo.renderModelName = "";
						componentInfo.isRenderable = false;

						m_renderComponentNames.push_back(componentInfo);
					}
				}

				delete[] szComponentName;
			}
		}
	}
	else
	{
		m_renderComponentNames.push_back({"", m_renderModelName });
	}

	return true;
}

std::string SteamVRDeviceProperties::fetchStringDeviceProperty(
	vr::ETrackedDeviceProperty property, 
	const std::string& default_string)
{
	vr::IVRSystem* vrSystem= vr::VRSystem();
	if (vrSystem == nullptr)
		return "";

	char szResult[512];
	vr::ETrackedPropertyError error = vr::TrackedProp_Success;

	vrSystem->GetStringTrackedDeviceProperty(
		m_deviceIndex,
		property,
		szResult,
		(uint32_t)sizeof(szResult),
		&error);

	return error == vr::TrackedProp_Success ? szResult : default_string;
}

std::string SteamVRDeviceProperties::fetchStringSettingsProperty(
	const char* sectionName,
	const char* sectionKey,
	const std::string& default_string)
{
	vr::IVRSettings* vrSettings= vr::VRSettings();
	if (vrSettings == nullptr)
		return "";

	char szResult[512];
	vr::EVRSettingsError error = vr::VRSettingsError_None;
	vrSettings->GetString(sectionName, sectionKey, szResult, (uint32_t)sizeof(szResult), &error);

	return error == vr::TrackedProp_Success ? szResult : default_string;
}