#pragma once

#include "NodePin.h"

class IntPinBase : public NodePin
{
public:
	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;
	virtual void editorRenderContextMenu(class NodeEditorState* editorState) override;
};

class IntPin : public IntPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;
	virtual void editorRenderBeginLink(float alpha) override;
	virtual ImU32 editorGetLinkStyleColor() const override;

protected:
	int value = 0;
};

class Int2Pin : public IntPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	int value[2]{};
};

class Int3Pin : public IntPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	int value[3]{};
};

class Int4Pin : public IntPinBase
{
public:
	virtual void editorRenderInputTextEntry(class NodeEditorState* editorState) override;

protected:
	int value[4]{};
};