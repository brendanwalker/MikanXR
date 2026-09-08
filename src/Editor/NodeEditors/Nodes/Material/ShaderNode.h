#pragma once

#include "Nodes/Node.h"
#include "Pins/ShaderValuePin.h"

class MaterialCompileContext;

// Base of every material graph node. A shader node never evaluates at
// runtime; it contributes shader text through compileNode and reports its
// outputs on the compile context.
class ShaderNode : public Node
{
public:
	ShaderNode()= default;

	inline static const std::string k_nodeClassName= "ShaderNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	// Resolve inputs through the context, emit code, and call context.setOutput
	// for every output pin. Return false after reporting an error.
	virtual bool compileNode(MaterialCompileContext& context)= 0;

	// Material graphs are compiled, never evaluated
	virtual bool evaluateNode(NodeEvaluator& evaluator) override { return true; }

	ShaderValuePinPtr getShaderInputPin(const std::string& pinName) const;
	ShaderValuePinPtr getShaderOutputPin(const std::string& pinName) const;
	ShaderValuePinPtr addShaderInputPin(const std::string& pinName, eShaderValueType type,
										const ShaderValueDefault& defaultValue= {});
	ShaderValuePinPtr addShaderOutputPin(const std::string& pinName, eShaderValueType type);

	// A warning glyph drawn on the node when it makes the material non-portable
	virtual bool editorShowsPortabilityWarning() const { return false; }

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
	// The default title band, followed by the portability warning glyph when the node asks for one
	virtual void editorRenderTitle(class MkCanvasScopedNode& scopedNode) const override;
	virtual void editorComputeNodeDimensions(NodeDimensions& outDims) const override;

	// Text of the warning glyph as rendered after the title, so width math and rendering agree
	static const char* editorGetPortabilityWarningSuffix();
};

using ShaderNodePtr= std::shared_ptr<ShaderNode>;
using ShaderNodeConstPtr= std::shared_ptr<const ShaderNode>;
