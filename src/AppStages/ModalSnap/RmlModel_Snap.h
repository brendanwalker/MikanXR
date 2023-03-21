#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_Snap : public RmlModel
{
public:
	inline void setSource(const int sourceFastenerId) { m_sourceFastenerId= sourceFastenerId; }

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;
	void setSourceFastenerId(int sourceFastenerId);
	void setTargetFastenerId(int targetFastenerId);
	void rebuildCompatibleTargetFasteners();


	SinglecastDelegate<void(int sourceFastenerId, int targetFastenerId)> OnRequestFastenerSnap;
	SinglecastDelegate<void()> OnCancelFastenerSnap;

protected:
	int m_sourceFastenerId;
	int m_targetFastenerId;
	Rml::Vector<int> m_compatibleTargetFastenerIds;
};
