#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "MikanCoreTypes.h"

class IMkTexture;
typedef std::shared_ptr<IMkTexture> IMkTexturePtr;

class SharedTextureReadAccessor
{
public:
	SharedTextureReadAccessor(const std::string& senderPrefix, MikanCameraID cameraId);
	~SharedTextureReadAccessor();

	bool initialize(const MikanRenderTargetDescriptor* descriptor);
	void dispose();
	void setColorTexture(IMkTexturePtr texture) { m_colorTexture= texture; }
	void setDepthTexture(IMkTexturePtr texture) { m_depthTexture= texture; }
	inline MikanCameraID getCameraId() const { return m_cameraId; }
	inline IMkTexturePtr getColorTexture() const { return m_colorTexture; }
	inline IMkTexturePtr getDepthTexture() const { return m_depthTexture; }
	inline int64_t getLastFrameRenderedIndex() const { return m_lastFrameRenderedIndex; }
	bool readRenderTargetTextures(const int64_t newFrameIndex);

	MikanClientGraphicsApi getClientGraphicsAPI() const { return m_descriptor.graphicsAPI; }
	inline const std::string& getColorSenderName() const { return m_colorSenderName; }
	inline const std::string& getDepthSenderName() const { return m_depthSenderName; }
	inline const MikanRenderTargetDescriptor& getRenderTargetDescriptor() const { return m_descriptor; }

private:
	std::string m_senderPrefix;
	MikanCameraID m_cameraId;
	MikanRenderTargetDescriptor m_descriptor;
	int64_t m_lastFrameRenderedIndex;
	std::string m_colorSenderName;
	std::string m_depthSenderName;
	IMkTexturePtr m_colorTexture;
	IMkTexturePtr m_depthTexture;
	struct RenderTargetReaderImpl* m_readerImpl;
};
using SharedTextureReadAccessorPtr= std::shared_ptr<SharedTextureReadAccessor>;