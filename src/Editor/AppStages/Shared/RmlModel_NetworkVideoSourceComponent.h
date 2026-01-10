#pragma once

#include "IVideoDevice.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_NetworkVideoSourceComponent : public RmlModel_MikanComponent
{
public:
	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;
};