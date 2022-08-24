// -- includes -----
#include "VRDeviceEnumerator.h"
#include "SteamVRDeviceEnumerator.h"

#include "assert.h"
#include "string.h"

// -- globals -----

// -- TrackerDeviceEnumerator -----
VRDeviceEnumerator::VRDeviceEnumerator()
	: DeviceEnumerator()
	, m_enumeratorIndex(0)
{
	m_enumerators.push_back({ eVRTrackerDeviceApi::STEAMVR, nullptr });

	allocateChildEnumerator();

	if (!isValid())
	{
		next();
	}
}

VRDeviceEnumerator::~VRDeviceEnumerator()
{
	for (auto& entry : m_enumerators)
	{
		delete entry.enumerator;
	}
}

const char* VRDeviceEnumerator::getDevicePath() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getDevicePath()
		: nullptr;
}

eDeviceType VRDeviceEnumerator::getDeviceType() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getDeviceType()
		: eDeviceType::INVALID;
}

int VRDeviceEnumerator::getUsbVendorId() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getUsbVendorId()
		: -1;
}

int VRDeviceEnumerator::getUsbProductId() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getUsbProductId()
		: -1;
}

eVRTrackerDeviceApi VRDeviceEnumerator::getVRTrackerApi() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].api_type
		: eVRTrackerDeviceApi::INVALID;
}

const SteamVRDeviceEnumerator* VRDeviceEnumerator::getSteamVRTrackerEnumerator() const
{
	return
		(getVRTrackerApi() == eVRTrackerDeviceApi::STEAMVR)
		? static_cast<SteamVRDeviceEnumerator*>(m_enumerators[m_enumeratorIndex].enumerator)
		: nullptr;
}

bool VRDeviceEnumerator::isValid() const
{
	if (m_enumeratorIndex < m_enumerators.size())
	{
		if (m_enumerators[m_enumeratorIndex].enumerator != nullptr)
		{
			return m_enumerators[m_enumeratorIndex].enumerator->isValid();
		}
	}

	return false;
}

bool VRDeviceEnumerator::next()
{
	bool foundValid = false;

	while (!foundValid && m_enumeratorIndex < m_enumerators.size())
	{
		if (isValid())
		{
			m_enumerators[m_enumeratorIndex].enumerator->next();
			foundValid = m_enumerators[m_enumeratorIndex].enumerator->isValid();
		}
		else
		{
			++m_enumeratorIndex;

			if (m_enumeratorIndex < m_enumerators.size())
			{
				allocateChildEnumerator();
				foundValid = m_enumerators[m_enumeratorIndex].enumerator->isValid();
			}
		}
	}

	return foundValid;
}

void VRDeviceEnumerator::allocateChildEnumerator()
{
	EnumeratorEntry& entry = m_enumerators[m_enumeratorIndex];

	switch (entry.api_type)
	{
	case eVRTrackerDeviceApi::STEAMVR:
		entry.enumerator = new SteamVRDeviceEnumerator;
		break;
	}
}