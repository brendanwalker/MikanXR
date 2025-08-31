#pragma once

#include "MikanTypeFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_SelectCamera : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	SinglecastDelegate<void(MikanCameraID)> OnOk;
	SinglecastDelegate<void()> OnCancel;

protected:
	Rml::Vector<int> m_cameraIdList;
	int m_selectedCameraIndex = 0;
};
