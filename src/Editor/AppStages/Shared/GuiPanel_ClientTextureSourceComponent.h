#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ClientTextureSourceComponent.h"

enum class eTextureSourceDisplayBufferType : int
{
	Color = 0,
	Depth = 1,
};

class GuiPanel_ClientTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onGui() override;

	inline eTextureSourceDisplayBufferType getDisplayBufferType() const
	{
		return m_displayBufferType;
	}

private:
	eTextureSourceDisplayBufferType m_displayBufferType = eTextureSourceDisplayBufferType::Color;
};
