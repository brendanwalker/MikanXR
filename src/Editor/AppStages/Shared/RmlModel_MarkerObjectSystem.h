#pragma once

#include "Shared/RmlModel_MikanObjectSystem.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_MarkerObjectSystem : public RmlModel_MikanObjectSystem
{
public:
	RmlModel_MarkerObjectSystem();

	virtual bool init(Rml::Context* rmlContext, MikanObjectSystemPtr objectSystem) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemConfigPtr getMarkerObjectSystemConfig() const;

private:
	RmlDataBinding_ArucoIdListPtr m_arucoIdList;
};
