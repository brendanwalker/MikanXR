#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "StageComponent.h"

class RmlModel_StageComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_StageComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	StageComponentPtr getStageComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingVolumeIdList;
};