#pragma once

#include "Constants_DepthMeshCapture.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_DepthMeshCapture : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCaptureFlag() const;
	void setBypassCaptureFlag(const bool bNewFlag);

	eDepthMeshCaptureMenuState getMenuState() const;
	void setMenuState(eDepthMeshCaptureMenuState newState);

	SinglecastDelegate<void()> OnContinueEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;

private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	bool m_bypassCaptureFlag = false;
};
