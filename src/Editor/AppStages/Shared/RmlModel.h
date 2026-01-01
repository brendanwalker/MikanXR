#pragma once

#include "Shared/RmlModelInterface.h"

#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/DataModelHandle.h>

#include <vector>

class RmlModel : public IRmlModel
{
public:
	RmlModel() = default;
	virtual ~RmlModel();

	Rml::DataModelConstructor init(Rml::Context* rmlContext, const Rml::String& modelName);

	// IRmlModel interface
	virtual Rml::Context* getContext() override;
	virtual Rml::DataModelHandle& getModelHandle() override;
	virtual void dispose() override;
	virtual void update() override;
	virtual void addModelUpdateCallback(std::function<void()> callback) override;

protected:
	Rml::Context* m_context= nullptr;
	Rml::DataModelHandle m_modelHandle;
	Rml::String m_modelName;
	std::vector<std::function<void()>> m_modelUpdateCallbacks;
};
