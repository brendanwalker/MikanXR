#pragma once

#include "ColliderQuery.h"
#include "ComponentFwd.h"
#include "MikanClientTypes.h"
#include "RendererFwd.h"
#include "StencilComponent.h"

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

	const glm::mat4 getModelXform() const;
	void setModelXform(const glm::mat4& xform);

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
	virtual void update() override;
	virtual void dispose() override;

	inline ModelStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(ModelStencilConfigPtr config);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransformProperty(const glm::mat4& xform) override;
	virtual void setStencilWorldTransformProperty(const glm::mat4& xform) override;

	// Selection Events
	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionSelected();
	void onInteractionUnselected();

protected:
	void updateSceneComponentTransform();

	ModelStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::vector<GlStaticMeshInstancePtr> m_wireframeMeshes;
};
