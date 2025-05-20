#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
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

using CameraMap = std::map<MikanCameraID, CameraComponentWeakPtr>;

class CameraObjectSystemConfig : public CommonConfig
{
public:
	CameraObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	CameraDefinitionPtr getCameraConfig(MikanCameraID cameraId) const;
	CameraDefinitionPtr getCameraConfigByName(const std::string& cameraName) const;
	MikanCameraID addNewCamera(const std::string& cameraName, const struct MikanTransform& xform);
	bool removeCamera(MikanCameraID anchorId);

	static const std::string k_cameraListPropertyId;
	std::vector<CameraDefinitionPtr> cameraList;

	MikanCameraID nextCameraId= 0;
};

class CameraObjectSystem : public MikanObjectSystem
{
public:
	static CameraObjectSystemPtr getSystem() { return s_cameraObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	CameraObjectSystemConfigConstPtr getCameraSystemConfigConst() const;
	CameraObjectSystemConfigPtr getCameraSystemConfig();

	const CameraMap& getCameraMap() const { return m_cameraComponents; }
	CameraComponentPtr getCameraById(MikanCameraID cameraId) const;
	CameraComponentPtr getCameraByName(const std::string& cameraName) const;
	CameraComponentPtr addNewCamera(const std::string& cameraName, const GlmTransform& xform);
	bool removeCamera(MikanCameraID anchorId);

protected:
	CameraComponentPtr createCameraObject(CameraDefinitionPtr cameraConfig);
	void disposeCameraObject(MikanCameraID cameraId);

	CameraMap m_cameraComponents;

	static CameraObjectSystemWeakPtr s_cameraObjectSystem;
};