#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "SceneComponent.h"

class GuiPanel_SceneComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_SceneComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual bool init() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	SceneComponentPtr getSceneComponent() const;

private:
	std::vector<int> m_compositorIdList;
};
