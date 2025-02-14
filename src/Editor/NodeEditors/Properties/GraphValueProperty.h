#pragma once

#include "MikanRendererFwd.h"
#include "GraphProperty.h"

template <typename t_data_type>
class TypedGraphValuePropertyConfig : public GraphPropertyConfig
{
public:
	TypedGraphValuePropertyConfig() = default;

	virtual configuru::Config writeToJSON() override
	{
		configuru::Config ret = GraphPropertyConfig::writeToJSON();
		ret["default_value"] = defaultValue;
		return ret;
	}

	virtual void readFromJSON(const configuru::Config& pt)
	{
		GraphPropertyConfig::readFromJSON(pt);
		defaultValue = pt.get_or<t_data_type>("default_value", t_data_type());
	}

	t_data_type defaultValue;
};

class GraphValueProperty : public GraphProperty
{
public:
	GraphValueProperty() = default;

	// Used to dynamically create pins in the VariableNode
	virtual std::string getPinClassName() const { return ""; }
	virtual void copyValueFromPin(ValuePinConstPtr pin) {}
	virtual void copyValueToPin(ValuePinPtr pin) const {}

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;

protected:
	// Override this in derived classes to render a custom value editor
	virtual void editorRenderValue(const class NodeEditorState& editorState) {}
};

template <typename t_data_type, class t_pin_class>
class TypedGraphValueProperty : public GraphValueProperty
{
public:
	TypedGraphValueProperty() = default;

	virtual std::string getPinClassName() const { return t_pin_class::k_pinClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig,const NodeGraphConfig& graphConfig) override
	{
		auto typedConfig = std::static_pointer_cast<const TypedGraphValuePropertyConfig<t_data_type> >(propConfig);

		m_defaultValue = typedConfig->defaultValue;
		m_value= m_defaultValue;

		return GraphValueProperty::loadFromConfig(propConfig, graphConfig);
	}

	virtual void saveToConfig(GraphPropertyConfigPtr config) const override
	{
		auto typedConfig = std::static_pointer_cast< TypedGraphValuePropertyConfig<t_data_type> >(config);

		typedConfig->defaultValue = m_defaultValue;

		GraphValueProperty::saveToConfig(config);
	}

	virtual void copyValueFromPin(ValuePinConstPtr pin) override 
	{
		auto typedPin = std::static_pointer_cast< const t_pin_class >(pin);

		m_value = typedPin->getValue();
	}

	virtual void copyValueToPin(ValuePinPtr pin) const override 
	{
		auto typedPin = std::static_pointer_cast< t_pin_class >(pin);

		typedPin->setValue(m_value);
	}

protected:
	t_data_type m_defaultValue;
	t_data_type m_value;
};