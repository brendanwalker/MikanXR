#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

class RmlModel_StageComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_StageComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	TrackingVolumeObjectSystemPtr getTrackingVolumeObjectSystem() const;
	TrackingVolumeObjectSystemConfigPtr getTrackingVolumeObjectSystemConfig() const;

private:
	RmlDataBinding_ComponentListPtr m_trackingVolumeIdList;
	TrackingVolumeObjectSystemWeakPtr m_trackingVolumeObjectSystem;
};

using RmlModel_StageComponentPtr = std::shared_ptr<RmlModel_StageComponent>;