#pragma once

#include "NodePin.h"

#include <array>

class IntPinBase : public NodePin
{
public:
	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) override;
};

class IntPin : public IntPinBase
{
public:
	int getValue() const { return value; }
	void setValue(int inValue) { value = inValue; }

	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	int value = 0;
};

class Int2Pin : public IntPinBase
{
public:
	const std::array<int, 2>& getValue() const { return value; }
	void setValue(const std::array<int, 2>& inValue) { value = inValue; }

	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<int, 2> value{};
};

class Int3Pin : public IntPinBase
{
public:
	const std::array<int, 3>& getValue() const { return value; }
	void setValue(const std::array<int, 3>& inValue) { value = inValue; }

	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<int, 3> value{};
};

class Int4Pin : public IntPinBase
{
public:
	const std::array<int, 4>& getValue() const { return value; }
	void setValue(const std::array<int, 4>& inValue) { value = inValue; }

	virtual void copyValueFromSourcePin() override;
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) override;

protected:
	std::array<int, 4> value{};
};