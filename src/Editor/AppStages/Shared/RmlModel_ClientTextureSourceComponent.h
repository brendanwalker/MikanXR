#pragma once

#include "Shared/RmlModel_MikanComponent.h"

enum class eTextureSourceDisplayBufferType : int
{
	Color = 0,
	Depth = 1,
};

class RmlModel_ClientTextureSourceComponent : public RmlModel_MikanComponent
{
public:
	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;

	inline eTextureSourceDisplayBufferType getDisplayBufferType() const 
	{ 
		return static_cast<eTextureSourceDisplayBufferType>(m_displayBufferType); 
	}

protected:
	int m_displayBufferType = static_cast<int>(eTextureSourceDisplayBufferType::Color);
};
