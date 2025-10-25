#pragma once

#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "SceneFwd.h"

class RmlModel_SceneAddCompositor : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		CompositorObjectSystemPtr compositorSystem, 
		SceneComponentPtr ownerScene);
	virtual void dispose() override;

	SinglecastDelegate<void(MikanCameraID)> OnOk;
	SinglecastDelegate<void()> OnCancel;

protected:
	Rml::Vector<int> m_compositorIdList;
	int m_selectedCompositorId = -1;
};
