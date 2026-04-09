#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "SceneComponent.h"

class GuiPanel_SceneComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_SceneComponent(AppStage* ownerAppStage);

	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	SceneComponentPtr getSceneComponent() const;

private:
	GuiDataSource_ComboBox m_compositorDataSource;
};
