#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "SceneComponent.h"

class GuiPanel_SceneComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_SceneComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void render(float deltaSeconds) override;

protected:
	SceneComponentPtr getSceneComponent() const;

private:
	std::vector<int> m_compositorIdList;
};
