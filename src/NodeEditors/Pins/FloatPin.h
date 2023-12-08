#pragma once

#include "NodePin.h"
#include <array>

class FloatPinBase : public NodePin
{
public:
	FloatPinBase() = default;
	FloatPinBase(NodePtr ownerNode) : NodePin(ownerNode) {}

	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
};

class FloatPin : public FloatPinBase
{
public:
	FloatPin() = default;
	FloatPin(NodePtr ownerNode) : FloatPinBase(ownerNode) {}

	float getValue() const { return value; }
	void setValue(float inValue) { value = inValue; }

	virtual size_t getDataSize() const { return sizeof(float); }
	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	float value= 0.f;
};

class Float2Pin : public FloatPinBase
{
public:
	Float2Pin() = default;
	Float2Pin(NodePtr ownerNode) : FloatPinBase(ownerNode) {}

	const std::array<float, 2>& getValue() const { return value; }
	void setValue(const std::array<float, 2>& inValue) { value = inValue; }

	virtual size_t getDataSize() const { return 2*sizeof(float); }
	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<float, 2> value{};
};

class Float3Pin : public FloatPinBase
{
public:
	Float3Pin() = default;
	Float3Pin(NodePtr ownerNode) : FloatPinBase(ownerNode) {}

	const std::array<float, 3>& getValue() const { return value; }
	void setValue(const std::array<float, 3>& inValue) { value = inValue; }

	virtual size_t getDataSize() const { return 3*sizeof(float); }
	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<float, 3> value{};
};

class Float4Pin : public FloatPinBase
{
public:
	Float4Pin() = default;
	Float4Pin(NodePtr ownerNode) : FloatPinBase(ownerNode) {}

	const std::array<float, 4>& getValue() const { return value; }
	void setValue(const std::array<float, 4>& inValue) { value = inValue; }

	virtual size_t getDataSize() const { return 4*sizeof(float); }
	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<float, 4> value{};
};