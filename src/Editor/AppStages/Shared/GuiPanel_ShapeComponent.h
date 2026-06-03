#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "ShapeComponent.h"

// Abstract base for shape component panels - renders parent dropdown + texture source picker
class GuiPanel_ShapeComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_ShapeComponent(AppStage* ownerAppStage);

	virtual void onConstruct() override;

protected:
	ShapeComponentPtr getShapeComponent() const;

private:
	GuiDataSource_ComboBox m_parentTransformDataSource;
	GuiDataSource_ComboBox m_textureSourceDataSource;
};

class GuiPanel_QuadShapeComponent : public GuiPanel_ShapeComponent
{
public:
	GuiPanel_QuadShapeComponent(AppStage* ownerAppStage) : GuiPanel_ShapeComponent(ownerAppStage) {}
	virtual bool init() override;
};

class GuiPanel_BoxShapeComponent : public GuiPanel_ShapeComponent
{
public:
	GuiPanel_BoxShapeComponent(AppStage* ownerAppStage) : GuiPanel_ShapeComponent(ownerAppStage) {}
	virtual bool init() override;
};

class GuiPanel_ModelShapeComponent : public GuiPanel_ShapeComponent
{
public:
	GuiPanel_ModelShapeComponent(AppStage* ownerAppStage) : GuiPanel_ShapeComponent(ownerAppStage) {}
	virtual bool init() override;
};
