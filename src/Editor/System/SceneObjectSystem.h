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

class SceneObjectSystemConfig : public CommonConfig
{
public:
	SceneObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class SceneObjectSystem : public MikanObjectSystem
{
public:
	static SceneObjectSystemPtr getSystem() { return s_sceneObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	SceneObjectSystemConfigConstPtr getSceneSystemConfigConst() const;
	SceneObjectSystemConfigPtr getSceneSystemConfig();

protected:
	static SceneObjectSystemWeakPtr s_sceneObjectSystem;
};