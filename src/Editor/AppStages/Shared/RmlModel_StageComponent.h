#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_StageComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_StageComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	TrackingVolumeObjectSystemPtr getTrackingVolumeObjectSystem() const;
	TrackingVolumeObjectSystemConfigPtr getTrackingVolumeObjectSystemConfig() const;
	StageComponentPtr getStageComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingVolumeIdList;
	TrackingVolumeObjectSystemWeakPtr m_trackingVolumeObjectSystem;
};