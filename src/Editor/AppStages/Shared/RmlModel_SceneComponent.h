#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "SceneComponent.h"

class RmlModel_SceneComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_SceneComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	SceneComponentPtr getSceneComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_compositorIdList;
};