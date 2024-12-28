#pragma once

#include <RmlUi/Core/DataModelHandle.h>

#include <memory>
#include <string>


class RmlDataBinding
{
public:
	RmlDataBinding() = default;
	virtual ~RmlDataBinding() {}

	virtual bool init(Rml::DataModelConstructor constructor);
	virtual void dispose() {}

protected:
	Rml::DataModelHandle m_modelHandle;
};