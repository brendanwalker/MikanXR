#pragma once

#include "NodePin.h"
#include <array>

class FloatPinBase : public NodePin
{
public:
	FloatPinBase() = default;

	virtual std::string getClassName() const override { return "FloatPinBase"; }
	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
};

class FloatPin : public FloatPinBase
{
public:
	FloatPin() = default;

	float getValue() const { return value; }
	void setValue(float inValue) { value = inValue; }

	virtual std::string getClassName() const override { return "FloatPin"; }
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

	const std::array<float, 2>& getValue() const { return value; }
	void setValue(const std::array<float, 2>& inValue) { value = inValue; }

	virtual std::string getClassName() const override { return "Float2Pin"; }
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

	const std::array<float, 3>& getValue() const { return value; }
	void setValue(const std::array<float, 3>& inValue) { value = inValue; }

	virtual std::string getClassName() const override { return "Float3Pin"; }
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

	const std::array<float, 4>& getValue() const { return value; }
	void setValue(const std::array<float, 4>& inValue) { value = inValue; }

	virtual std::string getClassName() const override { return "Float4Pin"; }
	virtual size_t getDataSize() const { return 4*sizeof(float); }
	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<float, 4> value{};
};