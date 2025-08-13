#pragma once

#include "IUsbVideoDevice.h"
#include "WMFDeviceInfo.h"
#include "WorkerThread.h"

#include <array>
#include <string>
#include <set>

#include <Mfidl.h>
#include <Mfapi.h>
#include <Mferror.h>
#include <Strmif.h>
#include <Shlwapi.h>

class WMFVideoFrameProcessor : public IMFSampleGrabberSinkCallback, public WorkerThread
{
public:
	WMFVideoFrameProcessor(
		int deviceIndex,
		const WMFDeviceFormatInfo& deviceFormat,
		class MikanUsbVideoDevice* listener);
	~WMFVideoFrameProcessor();

	HRESULT init(IMFMediaSource* pSource);
	void dispose();

	HRESULT startVideoFrameThread();
	void stopVideoFrameThread();

	inline bool getIsRunning() const { return m_bIsRunning; }

protected:
	virtual bool doWork() override;

	HRESULT CreateTopology(IMFMediaSource* pSource, IMFActivate* pSinkActivate, IMFTopology** ppTopo);
	HRESULT AddSourceNode(
		IMFTopology* pTopology,
		IMFMediaSource* pSource,
		IMFPresentationDescriptor* pPD,
		IMFStreamDescriptor* pSD,
		IMFTopologyNode** ppNode);
	HRESULT AddOutputNode(
		IMFTopology* pTopology,
		IMFActivate* pActivate,
		DWORD dwId,
		IMFTopologyNode** ppNode);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFClockStateSink methods
	STDMETHODIMP OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset) { return S_OK; }
	STDMETHODIMP OnClockStop(MFTIME hnsSystemTime) { return S_OK; }
	STDMETHODIMP OnClockPause(MFTIME hnsSystemTime) { return S_OK; }
	STDMETHODIMP OnClockRestart(MFTIME hnsSystemTime) { return S_OK; }
	STDMETHODIMP OnClockSetRate(MFTIME hnsSystemTime, float flRate) { return S_OK; }

	// IMFSampleGrabberSinkCallback methods
	STDMETHODIMP OnSetPresentationClock(IMFPresentationClock* pClock) { return S_OK; }
	STDMETHODIMP OnShutdown() { return S_OK; }
	STDMETHODIMP OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
		LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE* pSampleBuffer,
		DWORD dwSampleSize);

private:
	int m_deviceIndex;
	const WMFDeviceFormatInfo& m_deviceFormat;
	class MikanUsbVideoDevice* m_videoSourceListener;

	long m_referenceCount;

	IMFMediaSession* m_pSession;
	IMFTopology* m_pTopology;

	bool m_bIsRunning;
	int64_t m_sampleIndex;
};

class MikanUsbVideoDevice : public IUsbVideoDevice
{
public:
	MikanUsbVideoDevice(
		class MikanWMFVideoDeviceManager* ownerDeviceManager,
		const WMFDeviceInfo& deviceInfo);
	virtual ~MikanUsbVideoDevice();

	// -- Device Listener
	virtual void addListener(IUsbVideoDeviceListener* listener) override;
	virtual void removeListener(IUsbVideoDeviceListener* listener) override;

	// -- Device Properties
	virtual const char* getDevicePath() const override;
	virtual const char* getFriendlyName() const override;

	// -- Video Mode
	virtual size_t getAvailableVideoModesCount() const override;
	virtual bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const override;
	virtual int getVideoModeIndex() const override;
	virtual const char* getVideoModeName() const override;
	virtual bool setVideoModeByName(const char* szVideoModeName) override;
	virtual bool setVideoModeByIndex(size_t index) override;

	// -- Camera Settings
	virtual bool getCameraSettingConstraint(const eUsbCameraSettingType property_type, UsbCameraSettingConstraint& outConstraint) const override;
	virtual void setCameraSetting(const eUsbCameraSettingType property_type, int desired_value) override;
	virtual int getCameraSetting(const eUsbCameraSettingType property_type) const override;

	// -- Video Streaming
	virtual eUsbVideoStreamingStatus startVideoStream() override;
	virtual eUsbVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

	void notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo);

protected:
	bool getIsOpen() const { return m_ownerDeviceManager != nullptr; }
	bool open();
	void close();

	/*
	  See https://msdn.microsoft.com/en-us/library/windows/desktop/dd407328(v=vs.85).aspx
	  VideoProcAmp_Brightness		[-10k, 10k]
	  VideoProcAmp_Contrast			[0, 10k]
	  VideoProcAmp_Hue				[-180k, 180k]
	  VideoProcAmp_Saturation		[0, 10k]
	  VideoProcAmp_Sharpness		[0, 100]
	  VideoProcAmp_Gamma			[1, 500]
	  VideoProcAmp_ColorEnable		0=off, 1=on
	  VideoProcAmp_WhiteBalance		device dependent
	  VideoProcAmp_BacklightCompensation		0=off, 1=on
	  VideoProcAmp_Gain				device dependent
	*/
	bool setProcAmpProperty(VideoProcAmpProperty propId, long desired_value, bool bAuto);
	long getProcAmpProperty(VideoProcAmpProperty propId, bool* bIsAuto = nullptr) const;
	bool getProcAmpRange(VideoProcAmpProperty propId, UsbCameraSettingConstraint& constraint) const;

	/*
		https://msdn.microsoft.com/en-us/library/windows/desktop/dd318253(v=vs.85).aspx
		CameraControl_Pan			[-180, 180]
		CameraControl_Tilt			[-180, 180]
		CameraControl_Roll			[-180, 180]
		CameraControl_Zoom			[10, 600]
		CameraControl_Exposure		1/2^n seconds (example n=-3 is 1/8th seconds)
		CameraControl_Iris			units of f_stop*10
		CameraControl_Focus			 optimally focused target, in millimeters
	*/
	bool setCameraControlProperty(CameraControlProperty propId, long desired_value, bool bAuto);
	long getCameraControlProperty(CameraControlProperty propId, bool* bIsAuto = nullptr) const;
	bool getCameraControlRange(CameraControlProperty propId, UsbCameraSettingConstraint& constraint) const;

	void notifyVideoDeviceDisconnected();
	void notifyVideoModePropertiesChanged();

private:
	class MikanWMFVideoDeviceManager* m_ownerDeviceManager;
	WMFDeviceInfo m_deviceInfo;
	int m_currentVideoModeIndex= -1;
	std::array<UsbCameraSettingConstraint, (int)eUsbCameraSettingType::COUNT> m_videoPropertyConstraints;
	IMFMediaSource* m_mediaSource= nullptr;
	WMFVideoFrameProcessor* m_videoFrameProcessor= nullptr;

	std::set<IUsbVideoDeviceListener*> m_listeners;
};

