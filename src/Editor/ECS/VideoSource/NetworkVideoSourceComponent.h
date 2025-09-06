#pragma once

#include "INetworkVideoDevice.h"
#include "VideoSourceComponent.h"

extern const std::string* k_NetworkVideoProtocol;

class NetworkVideoSourceDefinition : public VideoSourceDefinition
{
public:
	NetworkVideoSourceDefinition();
	NetworkVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const MikanNetworkVideoSourceInfo& videoSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_addressPropertyId;
	inline const std::string& getAddress() const { return m_address; }
	void setAddress(const std::string& address);

	static const std::string k_pathPropertyId;
	inline const std::string& getPath() const { return m_path; }
	void setPath(const std::string& path);

	static const std::string k_protocolPropertyId;
	inline eNetworkVideoProtocol getProtocol() const { return m_protocol; }
	void setProtocol(eNetworkVideoProtocol protocol);

	static const std::string k_portPropertyId;
	inline int getPort() const { return m_port; }
	void setPort(int port);

	static bool parseUrl(
		const std::string& url, 
		eNetworkVideoProtocol& outProtocol,
		std::string& outAddress,
		int& outPort,
		std::string& outPath);

private:
	eNetworkVideoProtocol m_protocol;
	std::string m_address;
	std::string m_path;
	int m_port = 0;
};

class NetworkVideoSourceComponent : public VideoSourceComponent, public INetworkVideoDeviceListener
{
public:
	NetworkVideoSourceComponent(MikanObjectWeakPtr owner);

	virtual void dispose() override;

	inline NetworkVideoSourceDefinitionPtr getNetworkVideoSourceDefinition() const
	{
		return std::static_pointer_cast<NetworkVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	// -- Video Source Interface ----
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const override;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer) override;

	// -- INetworkVideoDeviceListener ----
	virtual void notifyVideoDeviceClosed(const class INetworkVideoDevice* device) override;
	virtual void notifyVideoModePropertiesChanged(const class INetworkVideoDevice* device) override;
	virtual void notifyVideoFrameReceived(const NetworkVideoFrameBuffer& bufferInfo) override;

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_calibrateIntrinsicsFunctionId;
	static const std::string k_testIntrinsicsFunctionId;
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void calibrateIntrinsics();
	void testIntrinsics();

protected:
	bool reallocateOpencvBufferState();
	void releaseOpencvBufferState();
	void recomputeCameraProjectionMatrix();

private:
	int64_t m_lastVideoFrameReadIndex;
	class OpenCVVideoFrameBuffer* m_opencv_buffer_state[MAX_PROJECTION_COUNT];
	INetworkVideoDevicePtr m_networkVideoDevice = nullptr;
	glm::mat4 m_projectionMatrix;
};