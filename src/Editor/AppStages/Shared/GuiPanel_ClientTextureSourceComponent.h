#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "Shared/GuiDataSource_StringList.h"
#include "ClientTextureSourceComponent.h"

enum class eTextureSourceDisplayBufferType : int
{
	Color= 0,
	Depth= 1,
};

class GuiPanel_ClientTextureSourceComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_ClientTextureSourceComponent(AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}
	virtual bool init() override;
	virtual void onGui() override;

	inline eTextureSourceDisplayBufferType getDisplayBufferType() const
	{
		return m_displayBufferType;
	}

private:
	eTextureSourceDisplayBufferType m_displayBufferType= eTextureSourceDisplayBufferType::Color;
	GuiDataSource_StringList m_displayBufferDataSource;
};
