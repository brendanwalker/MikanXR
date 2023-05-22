#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "SceneComponent.h"
#include "MikanClientTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class SceneComponent;
using SceneComponentWeakPtr = std::weak_ptr<SceneComponent>;

class AnchorConfig : public CommonConfig
{
public:
	AnchorConfig();
	AnchorConfig(
		MikanSpatialAnchorID anchorId,
		const std::string& anchorName,
		const MikanMatrix4f& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanSpatialAnchorID getAnchorId() const { return m_anchorInfo.anchor_id; }
	const MikanSpatialAnchorInfo& getAnchorInfo() const { return m_anchorInfo; }

	static const std::string k_anchorNamePropertyID;
	const glm::mat4 getAnchorXform() const;
	void setAnchorXform(const glm::mat4& xform);

	static const std::string k_anchorXformPropertyID;
	const std::string getAnchorName() const;
	void setAnchorName(const std::string& anchorName);

private:
	MikanSpatialAnchorInfo m_anchorInfo;
};

class AnchorComponent : public SceneComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline AnchorConfigPtr getConfig() const { return m_config; }
	void setConfig(AnchorConfigPtr config);

	virtual void setRelativeTransform(const GlmTransform& newRelativeXform) override;
	virtual void setWorldTransform(const glm::mat4& newWorldXform) override;

	virtual void setName(const std::string& name) override;

protected:
	AnchorConfigPtr m_config;

	SceneComponentWeakPtr m_sceneComponent;
	void applySceneComponentTransformToConfig(SceneComponentPtr sceneComponent);
};