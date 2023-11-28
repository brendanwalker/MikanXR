#pragma once

#include "NodePin.h"

class FloatPinBase : public NodePin
{
public:
	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) override;
};

class FloatPin : public FloatPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	float value= 0.f;
};

class Float2Pin : public FloatPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	float value[2]{};
};

class Float3Pin : public FloatPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	float value[3]{};
};

class Float4Pin : public FloatPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	float value[4]{};
};