#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "CompositorComponent.h"
#include "CameraObjectSystem.h"

class GuiPanel_CompositorComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_CompositorComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void render(float deltaSeconds) override;

protected:
	CameraObjectSystemPtr getCameraObjectSystem() const;
	CompositorComponentPtr getCompositorComponent() const;

private:
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
};

using GuiPanel_CompositorComponentPtr = std::shared_ptr<GuiPanel_CompositorComponent>;
