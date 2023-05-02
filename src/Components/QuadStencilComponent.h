#pragma once

#include "MikanClientTypes.h"
#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "StencilComponent.h"
#include "Transform.h"

#include <string>

#include "glm/ext/vector_float3.hpp"

class QuadStencilConfig : public CommonConfig
{
public:
	QuadStencilConfig();
	QuadStencilConfig(MikanStencilID stencilId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const MikanStencilQuad& getQuadInfo() const { return m_quadInfo; }
	void setQuadInfo(const MikanStencilQuad& quadInfo);

	MikanStencilID getStencilId() const { return m_quadInfo.stencil_id; }
	MikanSpatialAnchorID getParentAnchorId() const { return m_quadInfo.parent_anchor_id; }

	const glm::mat4 getQuadMat4() const;
	void setQuadMat4(const glm::mat4& xform);

	const GlmTransform getQuadTransform() const;
	void setQuadTransform(const GlmTransform& transform);

	const MikanVector3f getQuadXAxis() const { return m_quadInfo.quad_x_axis; }
	void setQuadXAxis(const MikanVector3f& xAxis);

	const MikanVector3f getQuadYAxis() const { return m_quadInfo.quad_y_axis; }
	void setQuadYAxis(const MikanVector3f& yAxis);

	const MikanVector3f getQuadNormal() const  { return m_quadInfo.quad_normal; }
	void setQuadNormal(const MikanVector3f& normal);

	const MikanVector3f getQuadCenter() const  { return m_quadInfo.quad_center; }
	void setQuadCenter(const MikanVector3f& center);

	float getQuadWidth() const { return m_quadInfo.quad_width; }
	void setQuadWidth(float width);

	float getQuadHeight() const { return m_quadInfo.quad_height; }
	void setQuadHeight(float height);

	bool getIsDoubleSided() const { return m_quadInfo.is_double_sided; }
	void setIsDoubleSided(bool flag);

	bool getIsDisabled() const { return m_quadInfo.is_disabled; }
	void setIsDisabled(bool flag);

	const std::string getStencilName() const { return m_quadInfo.stencil_name; }
	void setStencilName(const std::string& stencilName);

private:
	MikanStencilQuad m_quadInfo;
};

class QuadStencilComponent : public StencilComponent
{
public:
	QuadStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline QuadStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(QuadStencilConfigPtr config);

	virtual void setConfigTransform(const GlmTransform& transform) override;
	virtual const GlmTransform getConfigTransform() override;

protected:
	void updateBoxColliderExtents();

	QuadStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};