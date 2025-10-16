#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "StageComponent.h"

class RmlModel_StageComponent : public RmlModel_TypedMikanComponent<StageComponent>
{
public:
	RmlModel_StageComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	TrackingVolumeObjectSystemPtr getTrackingVolumeObjectSystem() const;
	TrackingVolumeObjectSystemConfigPtr getTrackingVolumeObjectSystemConfig() const;
	StageComponentPtr getStageComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingVolumeIdList;
	TrackingVolumeObjectSystemWeakPtr m_trackingVolumeObjectSystem;
};