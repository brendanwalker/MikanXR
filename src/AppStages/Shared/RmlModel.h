#pragma once

#include "RmlFwd.h"
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/DataModelHandle.h>

class RmlModel
{
public:
	RmlModel() = default;
	virtual ~RmlModel();

	inline Rml::Context* getContext() { return m_context; }

	Rml::DataModelConstructor init(Rml::Context* rmlContext, const Rml::String& modelName);
	virtual void dispose();
	virtual void update() {}

protected:
	Rml::Context* m_context= nullptr;
	Rml::DataModelHandle m_modelHandle;
	Rml::String m_modelName;
};
