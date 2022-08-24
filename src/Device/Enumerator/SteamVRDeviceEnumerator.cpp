#include "SteamVRDeviceEnumerator.h"
#include "App.h"
#include "VRDeviceManager.h"
#include "SteamVRManager.h"

#define MAX_STEAMVR_TRACKERS 16

SteamVRDeviceEnumerator::SteamVRDeviceEnumerator()
	: DeviceEnumerator()
	, m_devicePath("")
	, m_deviceType(eDeviceType::INVALID)
	, m_vendorId(0)
	, m_productId(0)
	, m_enumeratorIndex(-1)
{
	const SteamVRManager* steamVRMgr= App::getInstance()->getVRDeviceManager()->getSteamVRManager();
	
	// Get all of the generic tracker ids
	m_steamVRDeviceIds = steamVRMgr->getActiveDevices();

	next();
}

bool SteamVRDeviceEnumerator::isValid() const
{
	return m_enumeratorIndex < m_steamVRDeviceIds.size();
}

int SteamVRDeviceEnumerator::getSteamVRDeviceID() const
{
	return isValid() ? m_steamVRDeviceIds[m_enumeratorIndex] : -1;
}

bool SteamVRDeviceEnumerator::next()
{
	++m_enumeratorIndex;

	return tryFetchVRTrackerProperties();
}

bool SteamVRDeviceEnumerator::tryFetchVRTrackerProperties()
{
	if (isValid())
	{
		const SteamVRManager* steamVRMgr = App::getInstance()->getVRDeviceManager()->getSteamVRManager();
		const int deviceId= m_steamVRDeviceIds[m_enumeratorIndex];

		m_vendorId= steamVRMgr->getDeviceVendorId(deviceId);
		m_productId = steamVRMgr->getDeviceProductId(deviceId);
		m_devicePath= steamVRMgr->getDevicePath(deviceId);
		m_deviceType= steamVRMgr->getDeviceType(deviceId);
		return true;
	}

	return false;
}