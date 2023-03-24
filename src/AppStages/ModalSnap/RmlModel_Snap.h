#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_Snap : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, class ProfileConfig* profile, int sourceFastenerId);
	virtual void dispose() override;
	int getTargetFastenerId(int targetFastenerId) { return m_targetFastenerId; }

	SinglecastDelegate<void(int sourceFastenerId, int targetFastenerId)> OnRequestFastenerSnap;
	SinglecastDelegate<void()> OnCancelFastenerSnap;

protected:
	int m_sourceFastenerId;
	int m_targetFastenerId;
	Rml::Vector<int> m_compatibleTargetFastenerIds;
};
