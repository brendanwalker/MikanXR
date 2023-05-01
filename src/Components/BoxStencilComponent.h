#pragma once

#include "CommonConfig.h"
#include "StencilComponent.h"
#include "MikanClientTypes.h"
#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "Transform.h"

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

	const glm::mat4 getBoxMat4() const;
	void setBoxMat4(const glm::mat4& xform);

	const GlmTransform getBoxTransform() const;
	void setBoxTransform(const GlmTransform& transform);

	const MikanVector3f getBoxXAxis() const { return m_boxInfo.box_x_axis; }
	void setBoxXAxis(const MikanVector3f& xAxis);

	const MikanVector3f getBoxYAxis() const { return m_boxInfo.box_y_axis; }
	void setBoxYAxis(const MikanVector3f& yAxis);

	const MikanVector3f getBoxZAxis() const { return m_boxInfo.box_z_axis; }
	void setBoxZAxis(const MikanVector3f& zAxis);

	const MikanVector3f getBoxCenter() const { return m_boxInfo.box_center; }
	void setBoxCenter(const MikanVector3f& center);

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
	virtual void customRender() override;

	inline BoxStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(BoxStencilConfigPtr config);

protected:
	void onSceneComponentTranformChaged(SceneComponentPtr sceneComponentPtr) override;
	void updateSceneComponentTransform();
	void updateBoxColliderExtents();

	BoxStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};