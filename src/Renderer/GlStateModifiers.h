#pragma once

#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_bool4.hpp"

enum class eGlStencilFunction : int
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

enum class eGlStencilOp : int
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

enum class eGlBlendEquation : int
{
	ADD,
	SUBTRACT,
	REVERSE_SUBTRACT,
	MIN,
	MAX
};

enum class eGlBlendFunction : int
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

enum class eGlFrameBuffer : int
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

void glStateSetViewport(GlState& glState, int x, int y, int width, int height);
void glStateSetClearColor(GlState& glState, const glm::vec4& color);
void glStateSetColorMask(GlState& glState, const glm::bvec4& color_mask);
void glStateSetDepthMask(GlState& glState, bool depth_mask);

void glStateSetStencilBufferClearValue(GlState& glState, int value);
void glStateSetStencilMask(GlState& glState, uint32_t mask);
void glStateSetStencilFunc(GlState& glState, eGlStencilFunction func, int ref, uint32_t mask);
void glStateSetStencilOp(GlState& glState, 
						 eGlStencilOp stencil_fail, eGlStencilOp depth_fail, eGlStencilOp depth_stencil_pass);

void glStateSetBlendEquation(GlState& glState, eGlBlendEquation mode);
void glStateSetBlendFunc(GlState& glState, eGlBlendFunction source_factor, eGlBlendFunction dest_factor);

void glStateSetDrawBuffer(GlState& glState, eGlFrameBuffer mode);
void glStateSetReadBuffer(GlState& glState, eGlFrameBuffer mode);