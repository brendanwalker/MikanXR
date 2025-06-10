#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class CompositorDefinition : public MikanComponentDefinition
{
public:
	CompositorDefinition();
	CompositorDefinition(
		MikanCompositorID compositorId,
		const std::string& compositorName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanCompositorID getCompositorId() const { return m_compositorId; }

private:
	MikanCompositorID m_compositorId;
};

class CompositorComponent : public MikanComponent
{
public:
	CompositorComponent(MikanObjectWeakPtr owner);
	virtual void init() override;

	inline CompositorDefinitionPtr getCompositorDefinition() const
	{
		return std::static_pointer_cast<CompositorDefinition>(m_definition);
	}
};
