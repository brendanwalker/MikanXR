#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"

class GuiPanel_SpoutTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_SpoutTextureSourceComponent() = default;

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void update(float deltaSeconds) override;
	virtual void render(float deltaSeconds) override;

protected:
	SpoutTextureSourceComponentPtr getSpoutTextureSourceComponent() const;
	SpoutTextureSourceSystemPtr getSpoutTextureSourceSystem() const;

private:
	SpoutTextureSourceSystemWeakPtr m_spoutTextureSourceSystem;
	std::vector<std::string> m_spoutSenderNames;
	float m_timeSinceLastSourceListRefresh = 0.0f;
	static constexpr float k_spoutSourceListUpdateInterval = 3.0f;
};
