#include "RmlModel.h"
#include <RmlUi/Core/Context.h>
#include <assert.h>

RmlModel::~RmlModel()
{
	dispose();
}

Rml::DataModelConstructor RmlModel::init(Rml::Context* rmlContext, const Rml::String& modelName)
{
	m_context = rmlContext;
	m_modelName = modelName;

	Rml::DataModelConstructor constructor = rmlContext->CreateDataModel(modelName);
	if (constructor)
	{
		m_modelHandle = constructor.GetModelHandle();
	}
	else
	{
		// Did you create a model with a duplicate name?
		m_modelHandle = Rml::DataModelHandle(nullptr);
		assert(false);
	}

	return constructor;
}

void RmlModel::dispose()
{
	if (m_context != nullptr && m_modelHandle)
	{
		m_context->RemoveDataModel(m_modelName);
		m_modelHandle = Rml::DataModelHandle(nullptr);
		m_context= nullptr;
	}

	m_modelUpdateCallbacks.clear();
}

void RmlModel::update()
{
	for (auto& callback : m_modelUpdateCallbacks)
	{
		callback();
	}
	m_modelUpdateCallbacks.clear();
}

void RmlModel::addModelUpdateCallback(std::function<void()> callback)
{
	m_modelUpdateCallbacks.push_back(callback);
}