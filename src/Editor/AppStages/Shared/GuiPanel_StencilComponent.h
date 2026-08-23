#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "StencilComponent.h"

// Abstract base for stencil component panels - renders anchor dropdown + cull mode
class GuiPanel_StencilComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_StencilComponent(AppStage* ownerAppStage);

	virtual void onConstruct() override;

protected:
	StencilComponentPtr getStencilComponent() const;

private:
	GuiDataSource_ComboBox m_parentTransformDataSource;
};

class GuiPanel_QuadStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_QuadStencilComponent(AppStage* ownerAppStage)
		: GuiPanel_StencilComponent(ownerAppStage)
	{
	}
	virtual bool init() override;
};

class GuiPanel_BoxStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_BoxStencilComponent(AppStage* ownerAppStage)
		: GuiPanel_StencilComponent(ownerAppStage)
	{
	}
	virtual bool init() override;
};

class GuiPanel_ModelStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_ModelStencilComponent(AppStage* ownerAppStage)
		: GuiPanel_StencilComponent(ownerAppStage)
	{
	}
	virtual bool init() override;
};
