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
	QuadStencilConfig(const MikanStencilQuad& quadInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const MikanStencilQuad& getQuadInfo() const { return m_quadInfo; }

	MikanStencilID getStencilId() const { return m_quadInfo.stencil_id; }

	static const std::string k_quadParentAnchorPropertyId;
	MikanSpatialAnchorID getParentAnchorId() const { return m_quadInfo.parent_anchor_id; }
	void setParentAnchorId(MikanSpatialAnchorID anchorId);

	const glm::mat4 getQuadMat4() const;
	void setQuadMat4(const glm::mat4& xform);

	const GlmTransform getQuadTransform() const;
	void setQuadTransform(const GlmTransform& transform);

	static const std::string k_quadStencilXAxisPropertyId;
	const MikanVector3f getQuadXAxis() const { return m_quadInfo.quad_x_axis; }
	void setQuadXAxis(const MikanVector3f& xAxis);

	static const std::string k_quadStencilYAxisPropertyId;
	const MikanVector3f getQuadYAxis() const { return m_quadInfo.quad_y_axis; }
	void setQuadYAxis(const MikanVector3f& yAxis);

	static const std::string k_quadStencilNormalPropertyId;
	const MikanVector3f getQuadNormal() const  { return m_quadInfo.quad_normal; }
	void setQuadNormal(const MikanVector3f& normal);

	void setQuadOrientation(const glm::mat3& rotation);

	static const std::string k_quadStencilCenterPropertyId;
	const MikanVector3f getQuadCenter() const  { return m_quadInfo.quad_center; }
	void setQuadCenter(const MikanVector3f& center);

	static const std::string k_quadStencilWidthPropertyId;
	float getQuadWidth() const { return m_quadInfo.quad_width; }
	void setQuadWidth(float width);

	static const std::string k_quadStencilHeightPropertyId;
	float getQuadHeight() const { return m_quadInfo.quad_height; }
	void setQuadHeight(float height);

	void setQuadSize(float width, float height);

	static const std::string k_quadStencilDisabledPropertyId;
	bool getIsDoubleSided() const { return m_quadInfo.is_double_sided; }
	void setIsDoubleSided(bool flag);

	static const std::string k_quadStencilDoubleSidedPropertyId;
	bool getIsDisabled() const { return m_quadInfo.is_disabled; }
	void setIsDisabled(bool flag);

	static const std::string k_quadStencilNamePropertyId;
	const std::string getStencilName() const { return m_quadInfo.stencil_name; }
	void setStencilName(const std::string& stencilName);

private:
	void notifyStencilChanged();

	MikanStencilQuad m_quadInfo;
};

class QuadStencilComponent : public StencilComponent
{
public:
	QuadStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	virtual CommonConfigPtr getComponentConfig() const override { return m_config; }
	inline QuadStencilConfigPtr getConfig() const { return m_config; }
	void setConfig(QuadStencilConfigPtr config);

	virtual MikanStencilID getParentAnchorId() const override;
	virtual void onParentAnchorChanged(MikanSpatialAnchorID newParentId) override;

	void setRelativePosition(const glm::vec3& position);
	void setRelativeOrientation(const glm::mat3& rotation);
	virtual void setRelativeTransform(const GlmTransform& newRelativeXform) override;
	virtual void setWorldTransform(const glm::mat4& newWorldXform) override;

	virtual void setName(const std::string& name) override;

protected:
	void updateBoxColliderExtents();

	QuadStencilConfigPtr m_config;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};