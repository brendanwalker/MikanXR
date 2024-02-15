#pragma once

#include "GraphValueProperty.h"
#include "Pins/BoolPin.h"

// Use the TypedGraphValuePropertyConfig template for our config
using GraphBoolPropertyConfig = TypedGraphValuePropertyConfig<bool>;

// Extend the TypedGraphValueProperty template for our property
class GraphBoolProperty : public TypedGraphValueProperty<bool, BoolPin>
{
public:
	GraphBoolProperty() = default;

	inline static const std::string k_propertyClassName = "GraphBoolProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }
	virtual std::string editorGetTitle() const override { return "Bool"; }

	virtual void editorRenderValue(const class NodeEditorState& editorState) override;
};

// Use the TypedGraphPropertyFactory template to create our factory
using GraphBoolPropertyFactory = TypedGraphPropertyFactory<GraphBoolProperty, GraphBoolPropertyConfig>;