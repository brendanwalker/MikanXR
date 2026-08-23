#include "AppStage.h"
#include "RGBPixelGridComponent.h"
#include "Shared/GuiPanel_RGBPixelGridComponent.h"

#include "imgui.h"

bool GuiPanel_RGBPixelGridComponent::init() { return initTypedPropertyInterface<RGBPixelGridComponent>(); }

void GuiPanel_RGBPixelGridComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(RGBPixelGridDefinition::k_gridColumnsPropertyId,
										  [this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
										  {
											  RGBPixelGridComponentPtr comp= getRGBPixelGridComponent();
											  if (!comp)
												  return false;

											  int columns= comp->getRGBPixelGridDefinition()->getColumns();
											  if (ImGui::InputInt("Columns", &columns))
											  {
												  if (columns < 1)
													  columns= 1;
												  addDeferredGuiEvent(
													  [this, columns]()
													  {
														  if (auto c= getRGBPixelGridComponent())
															  c->getRGBPixelGridDefinition()->setColumns(columns);
													  });
											  }
											  return true;
										  });

	m_entityAccessor->setPropertyRenderer(RGBPixelGridDefinition::k_gridRowsPropertyId,
										  [this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
										  {
											  RGBPixelGridComponentPtr comp= getRGBPixelGridComponent();
											  if (!comp)
												  return false;

											  int rows= comp->getRGBPixelGridDefinition()->getRows();
											  if (ImGui::InputInt("Rows", &rows))
											  {
												  if (rows < 1)
													  rows= 1;
												  addDeferredGuiEvent(
													  [this, rows]()
													  {
														  if (auto c= getRGBPixelGridComponent())
															  c->getRGBPixelGridDefinition()->setRows(rows);
													  });
											  }
											  return true;
										  });
}

RGBPixelGridComponentPtr GuiPanel_RGBPixelGridComponent::getRGBPixelGridComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
		return std::static_pointer_cast<RGBPixelGridComponent>(component);
	return nullptr;
}
