#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "ClientTextureSourceComponent.h"
#include "SpoutTextureSourceComponent.h"

class RmlModel_ClientTextureSourceComponent :
	public RmlModel_TypedMikanComponent<ClientTextureSourceComponent>
{
};

class RmlModel_SpoutTextureSourceComponent :
	public RmlModel_TypedMikanComponent<SpoutTextureSourceComponent>
{
};