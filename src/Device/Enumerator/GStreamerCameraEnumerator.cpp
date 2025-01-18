#include "GStreamerCameraEnumerator.h"
#include "VideoSourceManager.h"

GStreamerCameraEnumerator::GStreamerCameraEnumerator()
	: DeviceEnumerator()
	, m_cameraURIList()
	, m_cameraIndex(-1)
{
	rebuildCameraURIList();
	next();
}

void GStreamerCameraEnumerator::rebuildCameraURIList()
{
	auto* videoSourceManager= VideoSourceManager::getInstance();

	if (videoSourceManager->getGStreamerModule() != nullptr)
	{
		m_cameraURIList = videoSourceManager->getConfig().videoSourceURIs;
	}
}

bool GStreamerCameraEnumerator::isValid() const
{
	return m_cameraIndex >= 0 && m_cameraIndex < (int)m_cameraURIList.size();
}

const char* GStreamerCameraEnumerator::getDevicePath() const
{
	return isValid() ? m_cameraURIList[m_cameraIndex].c_str() : nullptr;
}

eDeviceType GStreamerCameraEnumerator::getDeviceType() const
{
	return isValid() ? eDeviceType::MonoVideoSource : eDeviceType::INVALID;
}

bool GStreamerCameraEnumerator::next()
{
	++m_cameraIndex;

	return isValid();
}