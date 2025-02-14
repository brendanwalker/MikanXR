#pragma once

#include "EnumUtils.h"

#include "MkRendererExport.h"
#include "MkRendererFwd.h"

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_bool4.hpp"

enum class eMkFrontFaceMode : int
{
	CW,		// Clockwise wound triangles are front facing.
	CCW,	// Counter-clockwise wound triangles are front facing.
};

enum class eMkStencilFunction : int
{
	NEVER,
	LESS,
	LEQUAL,
	GREATER,
	GEQUAL,
	EQUAL,
	NOTEQUAL,
	ALWAYS
};

enum class eMkStencilOp : int
{
	KEEP,		// Keeps the current value.
	ZERO,		// Sets the stencil buffer value to 0.
	REPLACE,	// Sets the stencil buffer value to ref, as specified by glStencilFunc.
	INCR,		// Increments the current stencil buffer value. 
				//   Clamps to the maximum representable unsigned value.
	INCR_WRAP,	// Increments the current stencil buffer value. 
				//   Wraps stencil buffer value to zero when incrementing the maximum representable unsigned value.
	DECR,		// Decrements the current stencil buffer value. Clamps to 0.
	DECR_WRAP,	// Decrements the current stencil buffer value. 
				//   Wraps stencil buffer value to the maximum representable unsigned value 
				//   when decrementing a stencil buffer value of zero.
	INVERT		// Bitwise inverts the current stencil buffer value.
};

enum class eMkBlendEquation : int
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX
};

enum class eMkBlendFunction : int
{
	ZERO,
	ONE,
	SRC_COLOR,
	ONE_MINUS_SRC_COLOR,
	DST_COLOR,
	ONE_MINUS_DST_COLOR,
	SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	DST_ALPHA,
	ONE_MINUS_DST_ALPHA,
	CONSTANT_COLOR,
	ONE_MINUS_CONSTANT_COLOR,
	CONSTANT_ALPHA,
	ONE_MINUS_CONSTANT_ALPHA,
	SRC_ALPHA_SATURATE,
	SRC1_COLOR,
	ONE_MINUS_SRC1_COLOR,
	SRC1_ALPHA,
	ONE_MINUS_SRC1_ALPHA
};

enum class eMkFrameBuffer : int
{
	// Default FrameBuffer Objects
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	FRONT_LEFT,
	FRONT_RIGHT,
	BACK_LEFT,
	BACK_RIGHT,
	FRONT_AND_BACK,

	// User Created FrameBuffer Objects Only
	NONE,
	COLOR_ATTACHMENT0,
	COLOR_ATTACHMENT1,
	COLOR_ATTACHMENT2,
	COLOR_ATTACHMENT3,
	COLOR_ATTACHMENT4,
	COLOR_ATTACHMENT5,
	COLOR_ATTACHMENT6,
	COLOR_ATTACHMENT7,
	COLOR_ATTACHMENT8,
	COLOR_ATTACHMENT9,
	COLOR_ATTACHMENT10,
	COLOR_ATTACHMENT11,
	COLOR_ATTACHMENT12,
	COLOR_ATTACHMENT13,
	COLOR_ATTACHMENT14,
	COLOR_ATTACHMENT15
};

enum class eMkClearFlags : uint32_t
{
	none = 0,
	color = 1 << 0,
	depth = 1 << 1,
	stencil = 1 << 2,
};
DEFINE_ENUM_BITMASK_OPERATORS(eMkClearFlags);

MIKAN_RENDERER_FUNC(void) mkStateSetFrontFace(IMkState* mkState, eMkFrontFaceMode mode);

MIKAN_RENDERER_FUNC(void) mkStateSetViewport(IMkState* mkState, int x, int y, int width, int height);
MIKAN_RENDERER_FUNC(void) mkStateSetClearColor(IMkState* mkState, const glm::vec4& color);
MIKAN_RENDERER_FUNC(void) mkStateSetColorMask(IMkState* mkState, const glm::bvec4& color_mask);
MIKAN_RENDERER_FUNC(void) mkStateSetDepthMask(IMkState* mkState, bool depth_mask);

MIKAN_RENDERER_FUNC(void) mkStateSetStencilBufferClearValue(IMkState* mkState, int value);
MIKAN_RENDERER_FUNC(void) mkStateSetStencilMask(IMkState* mkState, uint32_t mask);
MIKAN_RENDERER_FUNC(void) mkStateSetStencilFunc(IMkState* mkState, eMkStencilFunction func, int ref, uint32_t mask);
MIKAN_RENDERER_FUNC(void) mkStateSetStencilOp(IMkState* mkState, 
						 eMkStencilOp stencil_fail, eMkStencilOp depth_fail, eMkStencilOp depth_stencil_pass);

MIKAN_RENDERER_FUNC(void) mkStateSetBlendEquation(IMkState* mkState, eMkBlendEquation mode);
MIKAN_RENDERER_FUNC(void) mkStateSetBlendFunc(IMkState* mkState, eMkBlendFunction source_factor, eMkBlendFunction dest_factor);

MIKAN_RENDERER_FUNC(void) mkStateSetDrawBuffer(IMkState* mkState, eMkFrameBuffer mode);
MIKAN_RENDERER_FUNC(void) mkStateSetReadBuffer(IMkState* mkState, eMkFrameBuffer mode);
MIKAN_RENDERER_FUNC(void) mkStateClearBuffer(IMkState* mkState, eMkClearFlags flags);