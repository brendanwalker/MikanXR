#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_ClientTextureSourceComponent : public RmlModel_MikanComponent
{
public:
	virtual bool init(Rml::Context* rmlContext) override;
};

class RmlModel_SpoutTextureSourceComponent : public RmlModel_MikanComponent
{
public:
	virtual bool init(Rml::Context* rmlContext) override;
};