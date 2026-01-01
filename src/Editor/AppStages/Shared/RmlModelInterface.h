#pragma once

#include "RmlFwd.h"

#include <functional>

class IRmlModel
{
public:
	IRmlModel() = default;
	virtual ~IRmlModel() = default;

	virtual Rml::Context* getContext() = 0;
	virtual Rml::DataModelHandle& getModelHandle() = 0;

	virtual void dispose()= 0;
	virtual void update()= 0;

	virtual void addModelUpdateCallback(std::function<void()> callback) = 0;
};