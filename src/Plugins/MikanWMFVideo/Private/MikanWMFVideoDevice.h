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
#include <mfreadwrite.h>
#include <Strmif.h>
#include <Shlwapi.h>
#include <wmcodecdsp.h>

class WMFVideoFrameProcessor : public IMFSourceReaderCallback
{
public:
	enum class State
	{
		Stopped,
		Starting,
		Running,
		Stopping,
		Failed
	};

	WMFVideoFrameProcessor(
		int deviceIndex,
		const WMFDeviceFormatInfo& deviceFormat,
		class MikanWMFVideoDevice* listener);
	~WMFVideoFrameProcessor();

	HRESULT init(IMFMediaSource* pSource);
	void dispose();

	void startVideoFrameStream();
	void stopVideoFrameStream();

	inline State getState() const { return m_state; }
	inline bool getIsRunning() const { return m_state == State::Running; }

protected:
	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(
		HRESULT hrStatus,
		DWORD dwStreamIndex,
		DWORD dwStreamFlags,
		LONGLONG llTimestamp,
		IMFSample* pSample);
	STDMETHODIMP OnFlush(DWORD dwStreamIndex) { return S_OK; }
	STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent) { return S_OK; }

private:
	HRESULT createDecoder();
	HRESULT getActualInputMediaType(IMFMediaType** ppMediaType);
	HRESULT findBestH264Decoder(CLSID* pDecoderCLSID);
	HRESULT processCompressedSample(IMFSample* pCompressedSample, IMFSample** ppDecodedSample);

	int m_deviceIndex;
	const WMFDeviceFormatInfo& m_deviceFormat;
	class MikanWMFVideoDevice* m_videoSourceListener;

	long m_referenceCount;

	IMFSourceReader* m_pSourceReader;
	IMFTransform* m_pDecoderTransform;
	IMFMediaType* m_pNativeInputType;  // Saved native H.264 media type from camera
	bool m_bNeedsDecoder;
	MFT_OUTPUT_STREAM_INFO m_decoderOutputInfo;

	State m_state;
	int64_t m_sampleIndex;
	eUSBVideoFrameBufferFormat m_outputFormat;
	GUID m_wmfOutputFormat;

	// NV12 plane offset tracking
	bool m_nv12_offsets_detected;
	size_t m_nv12_uv_plane_offset;
};

class MikanWMFVideoDevice : public IUsbVideoDevice
{
public:
	MikanWMFVideoDevice(
		class MikanWMFVideoDeviceManager* ownerDeviceManager,
		const WMFDeviceInfo& deviceInfo);
	virtual ~MikanWMFVideoDevice();

	// -- Device Listener
	virtual void addListener(IUsbVideoDeviceListener* listener) override;
	virtual void removeListener(IUsbVideoDeviceListener* listener) override;

	// -- Device Properties
	virtual const char* getDevicePath() const override;
	virtual const char* getFriendlyName() const override;

	// -- Device Activation
	virtual bool getIsOpen() const override;
	virtual bool open() override;
	virtual void close() override;

	// -- Video Mode
	virtual size_t getAvailableVideoModesCount() const override;
	virtual bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const override;
	virtual int getVideoModeIndex() const override;
	virtual const char* getVideoModeName() const override;
	virtual bool setVideoModeByName(const char* szVideoModeName) override;
	virtual bool setVideoModeByIndex(size_t index) override;

	// -- Camera Settings
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const override;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const override;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value) override;
	virtual int getVideoSetting(const eVideoSettingType property_type) const override;

	// -- Video Streaming
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

	void notifyVideoDeviceDisconnected();
	void notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo);

protected:
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
	bool getProcAmpRange(VideoProcAmpProperty propId, VideoSettingConstraint& constraint) const;

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
	bool getCameraControlRange(CameraControlProperty propId, VideoSettingConstraint& constraint) const;

	void notifyVideoModePropertiesChanged();

private:
	class MikanWMFVideoDeviceManager* m_ownerDeviceManager;
	WMFDeviceInfo m_deviceInfo;
	int m_currentVideoModeIndex= -1;
	std::array<VideoSettingConstraint, (int)eVideoSettingType::COUNT> m_videoPropertyConstraints;
	IMFMediaSource* m_mediaSource= nullptr;
	WMFVideoFrameProcessor* m_videoFrameProcessor= nullptr;

	std::set<IUsbVideoDeviceListener*> m_listeners;
};

