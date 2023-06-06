#pragma once

#include "MikanClientTypes.h"
#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "StencilComponent.h"
#include "Transform.h"

#include <string>

#include "glm/ext/vector_float3.hpp"

class QuadStencilDefinition : public StencilComponentDefinition
{
public:
	QuadStencilDefinition();
	QuadStencilDefinition(const MikanStencilQuad& quadInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	//const MikanStencilQuad& getQuadInfo() const { return m_quadInfo; }

	static const std::string k_quadStencilWidthPropertyId;
	float getQuadWidth() const { return m_quadWidth; }
	void setQuadWidth(float width);

	static const std::string k_quadStencilHeightPropertyId;
	float getQuadHeight() const { return m_quadHeight; }
	void setQuadHeight(float height);

	void setQuadSize(float width, float height);

	static const std::string k_quadStencilDoubleSidedPropertyId;
	bool getIsDoubleSided() const { return m_bIsDoubleSided; }
	void setIsDoubleSided(bool flag);

private:
	//void notifyStencilChanged();

	float m_quadWidth;
	float m_quadHeight;
	bool m_bIsDoubleSided;
};

class QuadStencilComponent : public StencilComponent
{
public:
	QuadStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline QuadStencilDefinitionPtr getQuadStencilDefinition() const { return m_definition; }

protected:
	void updateBoxColliderExtents();

	QuadStencilDefinitionPtr m_definition;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};