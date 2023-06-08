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

class AnchorDefinition : public SceneComponentDefinition
{
public:
	AnchorDefinition();
	AnchorDefinition(
		MikanSpatialAnchorID anchorId,
		const std::string& anchorName,
		const MikanTransform& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanSpatialAnchorID getAnchorId() const { return m_anchorId; }
	MikanSpatialAnchorInfo getAnchorInfo() const;

private:
	MikanSpatialAnchorID m_anchorId;
};

class AnchorComponent : public SceneComponent
{
public:
	AnchorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;

	inline AnchorDefinitionPtr getAnchorDefinition() const
	{
		return std::static_pointer_cast<AnchorDefinition>(m_definition);
	}

protected:
	SelectionComponentWeakPtr m_selectionComponent;
};