#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "CompositorComponent.h"
#include "CameraObjectSystem.h"

class GuiPanel_CompositorComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_CompositorComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	CameraObjectSystemPtr getCameraObjectSystem() const;
	CompositorComponentPtr getCompositorComponent() const;

private:
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
};

using GuiPanel_CompositorComponentPtr = std::shared_ptr<GuiPanel_CompositorComponent>;
