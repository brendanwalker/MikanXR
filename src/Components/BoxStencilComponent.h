#pragma once

#include "CommonConfig.h"
#include "StencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"

class BoxStencilConfig : public CommonConfig
{
public:
	BoxStencilConfig();
	BoxStencilConfig(MikanStencilID stencilId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const MikanStencilBox& getBoxInfo() const { return m_boxInfo; }
	void setBoxInfo(const MikanStencilBox& box);

	MikanStencilID getStencilId() const { return m_boxInfo.stencil_id; }
	MikanStencilID getParentAnchorId() const { return m_boxInfo.parent_anchor_id; }

	const glm::mat4 getBoxXform() const;
	void setBoxXform(const glm::mat4& xform);

	float getBoxXSize() const { return m_boxInfo.box_x_size; }
	void setBoxXSize(float size);

	float getBoxYSize() const { return m_boxInfo.box_y_size; }
	void setBoxYSize(float size);

	float getBoxZSize() const { return m_boxInfo.box_z_size; }
	void setBoxZSize(float size);

	bool getIsDisabled() const { return m_boxInfo.is_disabled; }
	void setIsDisabled(bool flag);

	const std::string getStencilName() const { return m_boxInfo.stencil_name; }
	void setStencilName(const std::string& stencilName);

private:
	MikanStencilBox m_boxInfo;
};

class BoxStencilComponent : public StencilComponent
{
public:
	BoxStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void update() override;

	inline BoxStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(BoxStencilConfigPtr config);

	virtual glm::mat4 getStencilLocalTransform() const override;
	virtual glm::mat4 getStencilWorldTransform() const override;
	virtual void setStencilLocalTransformProperty(const glm::mat4& xform) override;
	virtual void setStencilWorldTransformProperty(const glm::mat4& xform) override;

protected:
	void updateSceneComponentTransform();
	void updateBoxColliderExtents();

	BoxStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};