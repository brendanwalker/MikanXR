#pragma once

#include "ColliderQuery.h"
#include "ComponentFwd.h"
#include "MikanClientTypes.h"
#include "RendererFwd.h"
#include "StencilComponent.h"
#include "Transform.h"

#include <memory>
#include <string>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class ModelStencilConfig : public CommonConfig
{
public:
	ModelStencilConfig();
	ModelStencilConfig(MikanStencilID stencilId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const MikanStencilModel& getModelInfo() const { return m_modelInfo; }
	void setModelInfo(const MikanStencilModel& modelInfo);

	MikanStencilID getStencilId() const { return m_modelInfo.stencil_id; }
	MikanStencilID getParentAnchorId() const { return m_modelInfo.parent_anchor_id; }

	const glm::mat4 getModelMat4() const;
	void setModelMat4(const glm::mat4& xform);

	const GlmTransform getModelTransform() const;
	void setModelTransform(const GlmTransform& transform);

	const MikanVector3f getModelScale() const { return m_modelInfo.model_scale; }
	void setModelScale(const MikanVector3f& scale);

	const MikanRotator3f getModelRotator() const { return m_modelInfo.model_rotator; }
	void setModelRotator(const MikanRotator3f& rotator);

	const MikanVector3f getModelPosition() const { return m_modelInfo.model_position; }
	void setModelPosition(const MikanVector3f& position);

	const std::filesystem::path& getModelPath() const { return m_modelPath; }
	void setModelPath(const std::filesystem::path& path);

	bool getIsDisabled() const { return m_modelInfo.is_disabled; }
	void setIsDisabled(bool flag);

	const std::string getStencilName() const { return m_modelInfo.stencil_name; }
	void setStencilName(const std::string& stencilName);

private:
	MikanStencilModel m_modelInfo;
	std::filesystem::path m_modelPath;
};

class ModelStencilComponent : public StencilComponent
{
public:
	ModelStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void renderLines() const override;
	virtual void dispose() override;

	inline ModelStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(ModelStencilConfigPtr config);

	// Selection Events
	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionSelected();
	void onInteractionUnselected();

protected:
	void onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr) override;
	void updateSceneComponentTransform();

	ModelStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::vector<GlStaticMeshInstancePtr> m_wireframeMeshes;
};
