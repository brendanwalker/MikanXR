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

class CameraObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	CameraObjectSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_cameraListPropertyId;
	CameraDefinitionPtr getCameraConfig(MikanCameraID cameraId) const;
	CameraDefinitionPtr getCameraConfigByName(const std::string& cameraName) const;
	MikanCameraID addNewCamera(MikanStageID ownerStageId);
	MikanCameraID addNewCamera(
		const std::string& cameraName, 
		const struct MikanTransform& xform,
		MikanStageID ownerStageId);
	bool removeCamera(MikanCameraID anchorId);
	const std::vector<CameraDefinitionPtr>& getCameraList() const { return cameraList; }

protected:
	std::vector<CameraDefinitionPtr> cameraList;
	MikanCameraID m_nextCameraId= 0;
};

class CameraObjectSystem : public MikanObjectSystem
{
public:
	CameraObjectSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static CameraObjectSystemPtr getSystem() { return s_cameraObjectSystem.lock(); }

	inline static const std::string k_objectSystemClassName = "CameraObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	CameraObjectSystemDefinitionConstPtr getCameraSystemConfigConst() const;
	CameraObjectSystemDefinitionPtr getCameraSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	std::vector<MikanCameraID> getAllCameraIds() const;

	const CameraMap& getCameraMap() const { return m_cameraComponents; }
	CameraComponentPtr getCameraById(MikanCameraID cameraId) const;
	CameraComponentPtr getCameraByName(const std::string& cameraName) const;
	CameraComponentPtr addNewCamera(const MikanStageID ownerStageId);
	bool removeCamera(MikanCameraID anchorId);

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	CameraComponentPtr createCameraObject(CameraDefinitionPtr cameraConfig);
	void disposeCameraObject(MikanCameraID cameraId);

	CameraMap m_cameraComponents;

	static CameraObjectSystemWeakPtr s_cameraObjectSystem;
};