#pragma once

#include "NodePin.h"
#include "MaterialCompiler/ShaderValueType.h"

class ShaderValuePinConfig : public NodePinConfig
{
public:
	ShaderValuePinConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string valueType;
	ShaderValueDefault defaultValue= {};
};

// The one pin class of the material graph. A link carries a symbolic shader
// value rather than a runtime value, so the pin holds only its type and the
// literal it falls back to when unconnected. The declared type is what the
// node asked for (wildcard on math nodes); the resolved type is what the last
// compile inferred, and only drives the editor colors.
class ShaderValuePin : public NodePin
{
public:
	ShaderValuePin();

	inline static const std::string k_pinClassName= "ShaderValuePin";
	virtual std::string getClassName() const override { return k_pinClassName; }

	virtual bool loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config) override;
	virtual void saveToConfig(NodePinConfigPtr config) const override;

	inline void setDeclaredType(eShaderValueType type) { m_declaredType= type; }
	inline eShaderValueType getDeclaredType() const { return m_declaredType; }

	inline void setResolvedType(eShaderValueType type) { m_resolvedType= type; }
	// The declared type when it is concrete, otherwise whatever the last compile inferred
	eShaderValueType getResolvedType() const;

	inline void setDefaultValue(const ShaderValueDefault& value) { m_defaultValue= value; }
	inline const ShaderValueDefault& getDefaultValue() const { return m_defaultValue; }

	virtual size_t getDataSize() const override { return sizeof(ShaderValueDefault); }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const override;

	virtual float editorComputeInputWidth() const override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;
	virtual MkCanvas::PinIcon editorGetPinIcon() const override;
	virtual ImVec4 editorGetPinColor() const override;
	virtual ImU32 editorGetLinkStyleColor() const override;

	static ImVec4 getColorForType(eShaderValueType type);

protected:
	eShaderValueType m_declaredType= eShaderValueType::wildcard;
	eShaderValueType m_resolvedType= eShaderValueType::INVALID;
	ShaderValueDefault m_defaultValue= {};
};

using ShaderValuePinPtr= std::shared_ptr<ShaderValuePin>;
using ShaderValuePinConstPtr= std::shared_ptr<const ShaderValuePin>;
