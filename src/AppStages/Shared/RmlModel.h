#pragma once

#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/DataModelHandle.h>

namespace Rml
{
	class Context;
};

class RmlModel
{
public:
	RmlModel() = default;
	virtual ~RmlModel();

	Rml::DataModelConstructor init(Rml::Context* rmlContext, const Rml::String& modelName);
	virtual void dispose();
	virtual void update() {}

protected:
	Rml::Context* m_context= nullptr;
	Rml::DataModelHandle m_modelHandle;
	Rml::String m_modelName;
};