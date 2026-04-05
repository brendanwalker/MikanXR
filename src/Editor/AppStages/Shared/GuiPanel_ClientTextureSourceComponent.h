#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ClientTextureSourceComponent.h"

enum class eTextureSourceDisplayBufferType : int;

class GuiPanel_ClientTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	virtual bool init(class AppStage* ownerAppStage) override;
	virtual void onGui() override;

	inline eTextureSourceDisplayBufferType getDisplayBufferType() const
	{
		return static_cast<eTextureSourceDisplayBufferType>(m_displayBufferType);
	}

private:
	int m_displayBufferType = 0; // eTextureSourceDisplayBufferType::Color
};
