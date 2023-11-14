#pragma once

enum class EditorPinType
{
	FLOW,
	FLOAT, FLOAT2, FLOAT3, FLOAT4,
	INT, INT2, INT3, INT4,
	ARRAY, // TODO Support array
	BLOCK,
	TEXTURE,
	IMAGE
};

enum class EditorBlockPinType
{
	UNIFROM_BLOCK,
	BUFFER_BLOCK
};

enum class EditorNodeType
{
	NODE,
	EVENT,
	PROGRAM,
	FRAMEBUFFER,
	TEXTURE,
	IMAGE,
	BLOCK,
	PINGPONG,
	TIME,
	MOUSE_POS
};

enum class EditorEventNodeType
{
	INIT,
	FRAME
};

enum class EditorProgramDispatchType
{
	ARRAY,
	COMPUTE
};

enum class EditorPingPongNodeType
{
	BUFFER,
	IMAGE
};