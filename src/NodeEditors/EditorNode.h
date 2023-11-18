#pragma once

#include "EditorNodeConstants.h"
#include "NodeEditorFwd.h"
#include "RendererFwd.h"

#include <string>
#include <vector>
#include <stdint.h>

class EditorNode
{
public:
	EditorNode();

public:
	int id;
	EditorNodeType type;
	std::vector<EditorPinPtr> pinsIn;
	std::vector<EditorPinPtr> pinsOut;
	struct {
		float x;
		float y;
	} nodePos;
};

class EditorEventNode : public EditorNode
{
public:
	EditorEventNode();

public:
	EditorEventNodeType eventNodeType;
};

class EditorProgramNode : public EditorNode
{
public:
	EditorProgramNode();

public:
	GlProgramPtr target;
	GlFrameBufferPtr framebuffer;
	int attachmentsPinsStartId;
	EditorPinPtr flowIn;
	EditorPinPtr flowOut;
	EditorProgramDispatchType dispatchType;
	GLenum drawMode;
	int dispatchSize[3];
};

class EditorTextureNode : public EditorNode
{
public:
	EditorTextureNode() = default;

public:
	GlTexturePtr target;
};

class EditorImageNode : public EditorNode
{
public:
	int sizeX;
	int sizeY;
	GLuint texture;

public:
	EditorImageNode();
	virtual ~EditorImageNode();
};

class EditorBlockNode : public EditorNode
{
public:
	int size;
	GLuint ubo;
	GLuint ssbo;
	int ssboSize;

public:
	EditorBlockNode();
	virtual ~EditorBlockNode();
};

class EditorPingPongNode : public EditorNode
{
public:
	EditorPingPongNode();

	EditorPingPongNodeType pingpongType = EditorPingPongNodeType::BUFFER;
	int size = 0;
};