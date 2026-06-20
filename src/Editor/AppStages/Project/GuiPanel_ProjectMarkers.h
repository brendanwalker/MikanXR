#pragma once

#include "Shared/GuiPanel.h"
#include "Shared/GuiDataSource_ComboBox.h"
#include "MarkerObjectSystem.h"
#include "MkGuiStyle.h"
#include "ObjectSystemFwd.h"

#include <memory>

class GuiPanel_ProjectMarkers : public GuiPanel
{
public:
	GuiPanel_ProjectMarkers(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;
	virtual void dispose() override;

private:
	MarkerObjectSystemPtr getMarkerSystem() const;
	MarkerComponentPtr getSelectedMarker() const;
	void setSelectedMarkerId(MikanMarkerID markerId);

	class ProjectGuiPanelContext* m_context= nullptr;
	MarkerObjectSystemWeakPtr m_markerSystem;
	int m_selectedMarkerId= INVALID_MIKAN_ID;

	MkGuiStyleConstPtr m_defaultGuiStyle;
	std::unique_ptr<GuiDataSource_ComboBox> m_markerDataSource;
};
