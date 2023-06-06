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

class BoxStencilDefinition : public StencilComponentDefinition
{
public:
	BoxStencilDefinition();
	BoxStencilDefinition(const MikanStencilBox& box);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	//const MikanStencilBox& getBoxInfo() const { return m_boxInfo; }

	static const std::string k_boxStencilXSizePropertyId;
	float getBoxXSize() const { return m_boxSize.x; }
	void setBoxXSize(float size);

	static const std::string k_boxStencilYSizePropertyId;
	float getBoxYSize() const { return m_boxSize.y; }
	void setBoxYSize(float size);

	static const std::string k_boxStencilZSizePropertyId;
	float getBoxZSize() const { return m_boxSize.z; }
	void setBoxZSize(float size);

	void setBoxSize(float xSize, float ySize, float zSize);

private:
	MikanVector3f m_boxSize;
};

class BoxStencilComponent : public StencilComponent
{
public:
	BoxStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline BoxStencilDefinitionPtr getBoxStencilDefinition() const { return m_definition; }

protected:
	void updateBoxColliderExtents();

	BoxStencilDefinitionPtr m_definition;
	SelectionComponentWeakPtr m_selectionComponent;
	BoxColliderComponentWeakPtr m_boxCollider;
};