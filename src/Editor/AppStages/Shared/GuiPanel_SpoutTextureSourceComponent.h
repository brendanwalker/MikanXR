#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"

class GuiPanel_SpoutTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_SpoutTextureSourceComponent(AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	void update(float deltaSeconds);

	// -- IGuiPanel
	virtual bool init() override;
	virtual void onConstruct() override;

protected:
	SpoutTextureSourceComponentPtr getSpoutTextureSourceComponent() const;
	SpoutTextureSourceSystemPtr getSpoutTextureSourceSystem() const;

private:
	SpoutTextureSourceSystemWeakPtr m_spoutTextureSourceSystem;
	GuiDataSource_StringList m_spoutSenderDataSource;
	std::vector<std::string> m_spoutSenderNames;
	float m_timeSinceLastSourceListRefresh= 0.0f;
	static constexpr float k_spoutSourceListUpdateInterval= 3.0f;
};
