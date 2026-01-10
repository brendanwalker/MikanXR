#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_SpoutTextureSourceComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_SpoutTextureSourceComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual void update(float deltaSeconds) override;

protected:
	SpoutTextureSourceComponentPtr getSpoutTextureSourceComponent() const;
	SpoutTextureSourceSystemPtr getSpoutTextureSourceSystem() const;

private:
	RmlDataBinding_SpoutSourceListPtr m_spoutSourceList;
	float m_timeSinceLastSourceListRefresh = 0.0f;
	static constexpr float k_spoutSourceListUpdateInterval = 3.0f; // seconds
};
