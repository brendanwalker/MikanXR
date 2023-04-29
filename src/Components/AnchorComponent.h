#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
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

	const glm::mat4 getAnchorXform() const;
	void setAnchorXform(const glm::mat4& xform);

	const std::string getAnchorName() const;
	void setAnchorName(const std::string& anchorName);

private:
	MikanSpatialAnchorInfo m_anchorInfo;
};

class AnchorComponent : public MikanComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void dispose() override;

	inline AnchorConfigPtr getConfig() const { return m_config; }
	void setConfig(AnchorConfigPtr config);

	glm::mat4 getAnchorLocalTransform() const;
	glm::mat4 getAnchorWorldTransform() const;

	void setAnchorLocalTransform(const GlmTransform& localTransform);
	void setAnchorWorldTransform(const glm::mat4& worldMat);

	const std::string getAnchorName();
	void setAnchorName(const std::string& newAnchorName);

protected:
	AnchorConfigPtr m_config;

	SceneComponentWeakPtr m_sceneComponent;
	bool m_bIsApplyingConfigTransform= false;
	void applyConfigTransformToSceneComponent();
	void applySceneComponentTransformToConfig(SceneComponentPtr sceneComponent);
};