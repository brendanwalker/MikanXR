#pragma once

#include "LocText.h"
#include "ShaderNode.h"
#include "MaterialCompiler/MaterialDomain.h"

// The single root of a material graph. Its input pins are the material's
// outputs: every domain has a color, the shape domain adds an object-space
// position offset that compiles into the vertex stage.
class MaterialOutputNode : public ShaderNode
{
public:
	MaterialOutputNode()= default;

	inline static const std::string k_nodeClassName= "MaterialOutputNode";
	inline static const std::string k_colorPinName= "color";
	inline static const std::string k_positionOffsetPinName= "positionOffset";

	virtual std::string getClassName() const override { return k_nodeClassName; }

	// Compiled through its pins by MaterialCompiler, never as a node
	virtual bool compileNode(MaterialCompileContext& context) override { return true; }

	// Recreate the input pins for a domain, dropping links the new set cannot carry
	void rebuildPinsForDomain(eMaterialDomain domain);

	ShaderValuePinPtr getColorPin() const { return getShaderInputPin(k_colorPinName); }
	ShaderValuePinPtr getPositionOffsetPin() const { return getShaderInputPin(k_positionOffsetPinName); }

	virtual bool editorCanDelete() const override { return false; }
	virtual std::string editorGetTitle() const override { return locText("nodes.materialOutputTitle"); }
	virtual const char* editorGetHeaderIcon() const override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
};
using MaterialOutputNodePtr= std::shared_ptr<MaterialOutputNode>;

class MaterialOutputNodeFactory : public TypedNodeFactory<MaterialOutputNode, NodeConfig>
{
public:
	MaterialOutputNodeFactory()= default;

	// Created once by the graph factory, never from the create menu
	virtual bool editorCanCreate() const override { return false; }
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
