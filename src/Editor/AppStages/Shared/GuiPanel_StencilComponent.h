#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "StencilComponent.h"
#include "AnchorObjectSystem.h"

// Abstract base for stencil component panels - renders anchor dropdown + cull mode
class GuiPanel_StencilComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_StencilComponent(AppStage* ownerAppStage) : GuiPanel_MikanComponent(ownerAppStage) {}

	virtual void onConstruct() override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onGui() override;

protected:
	StencilComponentPtr getStencilComponent() const;
	void rebuildTransformIdList();

private:
	std::vector<int> m_transformIdList;  // cached list of transform IDs for the dropdown
};

class GuiPanel_QuadStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_QuadStencilComponent(AppStage* ownerAppStage) : GuiPanel_StencilComponent(ownerAppStage) {}
	virtual bool init() override;
};

class GuiPanel_BoxStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_BoxStencilComponent(AppStage* ownerAppStage) : GuiPanel_StencilComponent(ownerAppStage) {}
	virtual bool init() override;
};

class GuiPanel_ModelStencilComponent : public GuiPanel_StencilComponent
{
public:
	GuiPanel_ModelStencilComponent(AppStage* ownerAppStage) : GuiPanel_StencilComponent(ownerAppStage) {}
	virtual bool init() override;
};
