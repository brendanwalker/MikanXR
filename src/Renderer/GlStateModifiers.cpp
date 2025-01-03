#include "GlCommon.h"
#include "GlStateStack.h"
#include "GlStateModifiers.h"
#include "GlViewport.h"
#include "IGlStateModifier.h"
#include "IGlWindow.h"

#include "memory"
#include "vector"

// -- GlStateModifierBase --
class GLStateModifierBase : public IGlStateModifier
{
public:
	GLStateModifierBase() = delete;
	GLStateModifierBase(GlState& glState)
	: m_ownerGlState(glState)
	, m_ownerStateStackDepth(glState.getStackDepth())
	{}

	inline GlState& getOwnerGlState() { return m_ownerGlState; }
	inline GlStateStack& getOwnerGlStateStack() { return m_ownerGlState.getOwnerStateStack(); }
	inline IGlWindow* getOwnerWindow() { return getOwnerGlStateStack().getOwnerWindow(); }
	virtual int getOwnerStateStackDepth() const override { return m_ownerStateStackDepth; }
	GlStateLog getStateLog()
	{
		return GlStateLog(m_ownerGlState);
	}

protected:
	GlState& m_ownerGlState;
	int m_ownerStateStackDepth;
};

// -- GlStateSetViewport --
class GLStateSetViewportImpl : public GLStateModifierBase
{
public:
	GLStateSetViewportImpl() = delete;
	GLStateSetViewportImpl(GlState& glState, int x, int y, int width, int height)
		: GLStateModifierBase(glState)
		, m_prevX(x), m_prevY(y), m_prevWidth(width), m_prevHeight(height)
		, m_x(x), m_y(y), m_width(width), m_height(height)
	{

	}

	inline static const std::string k_modifierID = "SetViewport";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentViewportModifier = std::static_pointer_cast<GLStateSetViewportImpl>(parentModifier);
		if (parentViewportModifier)
		{
			m_prevX = parentViewportModifier->m_x;
			m_prevY = parentViewportModifier->m_y;
			m_prevWidth = parentViewportModifier->m_width;
			m_prevHeight = parentViewportModifier->m_height;
		}
		else
		{
			GLint m_lastiewport[4];
			glGetIntegerv(GL_VIEWPORT, m_lastiewport);

			m_prevX= m_lastiewport[0]; 
			m_prevY= m_lastiewport[1];
			m_prevWidth= m_lastiewport[2];
			m_prevHeight= m_lastiewport[3];
		}

		glViewport(m_x, m_y, m_width, m_height);
		getStateLog() << "Apply Viewport: " << m_x << ", " << m_y << ", " << m_width << ", " << m_height;

		// Tell the owner window that we are applying new viewport bounds
		GlViewportPtr viewport= getOwnerWindow()->getRenderingViewport();
		if (viewport)
		{
			viewport->onRenderingViewportApply(m_x, m_y, m_width, m_height);
		}
	}

	virtual void revert() override
	{
		glViewport(m_prevX, m_prevY, m_prevWidth, m_prevHeight);
		getStateLog() << "Revert Viewport: " << m_prevX << ", " << m_prevY << ", " << m_prevWidth << ", " << m_prevHeight;

		// Tell the owner window that we are restoring previous viewport bounds
		GlViewportPtr viewport = getOwnerWindow()->getRenderingViewport();
		if (viewport)
		{
			viewport->onRenderingViewportRevert(m_x, m_y, m_width, m_height);
		}
	}

private:
	int32_t m_prevX, m_prevY, m_prevWidth, m_prevHeight;
	int32_t m_x, m_y, m_width, m_height;
};

void glStateSetViewport(GlState& glState, int x, int y, int width, int height)
{
	glState.addModifier(std::make_shared<GLStateSetViewportImpl>(glState, x, y, width, height));
}

// -- GLStateSetClearColor --
class GLStateSetClearColorImpl : public GLStateModifierBase
{
public:
	GLStateSetClearColorImpl() = delete;
	GLStateSetClearColorImpl(GlState& glState, const glm::vec4& color)
		: GLStateModifierBase(glState)
		, m_prevClearColor(color)
		, m_clearColor(color)
	{}

	inline static const std::string k_modifierID = "SetBackgroundColor";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentBackgroundColorModifier = std::static_pointer_cast<GLStateSetClearColorImpl>(parentModifier);
		if (parentBackgroundColorModifier)
		{
			m_prevClearColor = parentBackgroundColorModifier->m_clearColor;
		}
		else
		{
			GLfloat m_lastClearColor[4];
			glGetFloatv(GL_COLOR_CLEAR_VALUE, m_lastClearColor);

			m_prevClearColor.r = m_lastClearColor[0];
			m_prevClearColor.g = m_lastClearColor[1];
			m_prevClearColor.b = m_lastClearColor[2];
			m_prevClearColor.a = m_lastClearColor[3];
		}

		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		getStateLog() << "Apply Clear Color: " 
			<< m_clearColor.r << ", " << m_clearColor.g << ", " 
			<< m_clearColor.b << ", " << m_clearColor.a;
	}
	virtual void revert() override
	{
		glClearColor(m_prevClearColor.r, m_prevClearColor.g, m_prevClearColor.b, m_prevClearColor.a);
		getStateLog() << "Revert Clear Color: "
			<< m_prevClearColor.r << ", " << m_prevClearColor.g << ", " 
			<< m_prevClearColor.b << ", " << m_prevClearColor.a;
	}

private:
	glm::vec4 m_prevClearColor;
	glm::vec4 m_clearColor;
};

void glStateSetClearColor(GlState& glState, const glm::vec4& color)
{
	glState.addModifier(std::make_shared<GLStateSetClearColorImpl>(glState, color));
}

// -- GLStateSetColorMask --
class GLStateSetColorMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetColorMaskImpl() = delete;
	GLStateSetColorMaskImpl(GlState& glState, const glm::bvec4& colorMask)
		: GLStateModifierBase(glState)
		, m_prevColorMask(colorMask)
		, m_colorMask(colorMask)
	{}

	inline static const std::string k_modifierID = "SetColorMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentBackgroundColorModifier = std::static_pointer_cast<GLStateSetColorMaskImpl>(parentModifier);
		if (parentBackgroundColorModifier)
		{
			m_prevColorMask = parentBackgroundColorModifier->m_colorMask;
		}
		else
		{
			GLboolean lastColorMask[4];
			glGetBooleanv(GL_COLOR_WRITEMASK, lastColorMask);

			m_prevColorMask[0] = lastColorMask[0] == GL_TRUE;
			m_prevColorMask[1] = lastColorMask[1] == GL_TRUE;
			m_prevColorMask[2] = lastColorMask[2] == GL_TRUE;
			m_prevColorMask[3] = lastColorMask[3] == GL_TRUE;
		}

		glColorMask(
			m_colorMask[0] ? GL_TRUE : GL_FALSE, 
			m_colorMask[1] ? GL_TRUE : GL_FALSE, 
			m_colorMask[2] ? GL_TRUE : GL_FALSE, 
			m_colorMask[3] ? GL_TRUE : GL_FALSE);
		getStateLog() << "Apply Color Write Mask: "
			<< m_colorMask[0] << ", " << m_colorMask[1] << ", "
			<< m_colorMask[2] << ", " << m_colorMask[2];
	}
	virtual void revert() override
	{
		glColorMask(
			m_prevColorMask[0] ? GL_TRUE : GL_FALSE, 
			m_prevColorMask[1] ? GL_TRUE : GL_FALSE, 
			m_prevColorMask[2] ? GL_TRUE : GL_FALSE, 
			m_prevColorMask[3] ? GL_TRUE : GL_FALSE);
		getStateLog() << "Revert Color Write Mask: "
			<< m_prevColorMask[0] << ", " << m_prevColorMask[1] << ", "
			<< m_prevColorMask[2] << ", " << m_prevColorMask[2];
	}

private:
	glm::bvec4 m_prevColorMask;
	glm::bvec4 m_colorMask;
};

void glStateSetColorMask(GlState& glState, const glm::bvec4& color_mask)
{
	glState.addModifier(std::make_shared<GLStateSetColorMaskImpl>(glState, color_mask));
}

// -- GLStateSetDepthMask --
class GLStateSetDepthMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetDepthMaskImpl() = delete;
	GLStateSetDepthMaskImpl(GlState& glState, bool depthMask)
		: GLStateModifierBase(glState)
		, m_prevDepthMask(depthMask)
		, m_depthMask(depthMask)
	{}

	inline static const std::string k_modifierID = "SetDepthMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier = std::static_pointer_cast<GLStateSetDepthMaskImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevDepthMask = parentTypedModifier->m_depthMask;
		}
		else
		{
			GLboolean lastDepthMask;
			glGetBooleanv(GL_DEPTH_WRITEMASK, &lastDepthMask);

			m_prevDepthMask = lastDepthMask == GL_TRUE;
		}

		glDepthMask(m_depthMask ? GL_TRUE : GL_FALSE);
		getStateLog() << "Apply Depth Write Mask: " << m_depthMask;
	}
	virtual void revert() override
	{
		glDepthMask(m_prevDepthMask ? GL_TRUE : GL_FALSE);
		getStateLog() << "Revert Depth Write Mask: " << m_prevDepthMask;
	}

private:
	bool m_prevDepthMask;
	bool m_depthMask;
};

void glStateSetDepthMask(GlState& glState, bool depth_mask)
{
	glState.addModifier(std::make_shared<GLStateSetDepthMaskImpl>(glState, depth_mask));
}

// -- GLStateSetStencilBufferClearValue --
class GLStateSetStencilBufferClearValueImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilBufferClearValueImpl() = delete;
	GLStateSetStencilBufferClearValueImpl(GlState& glState, const int value)
		: GLStateModifierBase(glState)
		, m_prevValue(value)
		, m_value(value)
	{}

	inline static const std::string k_modifierID = "SetStencilBufferClearValue";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier = 
			std::static_pointer_cast<GLStateSetStencilBufferClearValueImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevValue = parentTypedModifier->m_value;
		}
		else
		{
			glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &m_prevValue);
		}

		glClearStencil(m_value);
		getStateLog() << "Apply Stencil Clear Value: " << m_value;
	}
	virtual void revert() override
	{
		glClearStencil(m_prevValue);
		getStateLog() << "Revert Stencil Clear Value: " << m_prevValue;
	}

private:
	GLint m_prevValue;
	GLint m_value;
};

void glStateSetStencilBufferClearValue(GlState& glState, int value)
{
	glState.addModifier(std::make_shared<GLStateSetStencilBufferClearValueImpl>(glState, value));
}

// -- GLStateSetStencilMaskImpl --
class GLStateSetStencilMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilMaskImpl() = delete;
	GLStateSetStencilMaskImpl(GlState& glState, const uint32_t mask)
		: GLStateModifierBase(glState)
		, m_prevMask(mask)
		, m_mask(mask)
	{}

	inline static const std::string k_modifierID = "SetStencilMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetStencilMaskImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevMask = parentTypedModifier->m_mask;
		}
		else
		{
			glGetIntegerv(GL_STENCIL_WRITEMASK, (GLint *)(&m_prevMask));
		}

		glStencilMask(m_mask);
		getStateLog() << "Apply Stencil Write Mask: " << m_mask;
	}
	virtual void revert() override
	{
		glStencilMask(m_prevMask);
		getStateLog() << "Revert Stencil Write Mask: " << m_prevMask;
	}

private:
	GLuint m_prevMask;
	GLuint m_mask;
};

void glStateSetStencilMask(GlState& glState, uint32_t mask)
{
	glState.addModifier(std::make_shared<GLStateSetStencilMaskImpl>(glState, mask));
}

// -- GLStateSetStencilFunc --
class GLStateSetStencilFuncImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilFuncImpl() = delete;
	GLStateSetStencilFuncImpl(GlState& glState, eGlStencilFunction func, int ref, uint32_t mask)
		: GLStateModifierBase(glState)
		, m_prevFunc(convertToGLenum(func))
		, m_func(m_prevFunc)
		, m_prevRef(ref)
		, m_ref(ref)
		, m_prevMask(mask)
		, m_mask(mask)
	{}

	inline static const std::string k_modifierID = "SetStencilFunc";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetStencilFuncImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevFunc = parentTypedModifier->m_func;
			m_prevRef = parentTypedModifier->m_ref;
			m_prevMask = parentTypedModifier->m_mask;
		}
		else
		{
			glGetIntegerv(GL_STENCIL_FUNC, (GLint*)(&m_prevFunc));
			glGetIntegerv(GL_STENCIL_REF, &m_prevRef);
			glGetIntegerv(GL_STENCIL_VALUE_MASK, (GLint*)(&m_prevMask));
		}

		glStencilFunc(m_func, m_ref, m_mask);
		getStateLog() << "Apply Stencil Function: " << m_func << ", " << m_ref << ", " << m_mask;
	}
	virtual void revert() override
	{
		glStencilFunc(m_prevFunc, m_prevRef, m_prevMask);
		getStateLog() << "Revert Stencil Function: " << m_prevFunc << ", " << m_prevRef << ", " << m_prevMask;
	}

	static GLenum convertToGLenum(eGlStencilFunction func)
	{
		switch (func)
		{
		case eGlStencilFunction::NEVER: return GL_NEVER;
		case eGlStencilFunction::LESS: return GL_LESS;
		case eGlStencilFunction::LEQUAL: return GL_LEQUAL;
		case eGlStencilFunction::GREATER: return GL_GREATER;
		case eGlStencilFunction::GEQUAL: return GL_GEQUAL;
		case eGlStencilFunction::EQUAL: return GL_EQUAL;
		case eGlStencilFunction::NOTEQUAL: return GL_NOTEQUAL;
		case eGlStencilFunction::ALWAYS: return GL_ALWAYS;
		}

		return GL_ALWAYS;
	}

private:
	GLenum m_prevFunc;
	GLenum m_func;
	GLint m_prevRef;
	GLint m_ref;
	GLuint m_prevMask;
	GLuint m_mask;
};

void glStateSetStencilFunc(GlState& glState, eGlStencilFunction func, int ref, uint32_t mask)
{
	glState.addModifier(std::make_shared<GLStateSetStencilFuncImpl>(glState, func, ref, mask));
}

// -- GLStateSetStencilOp --
class GLStateSetStencilOpImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilOpImpl() = delete;
	GLStateSetStencilOpImpl(
		GlState& glState, 
		eGlStencilOp stencil_fail, 
		eGlStencilOp depth_fail, 
		eGlStencilOp depth_stencil_pass)
		: GLStateModifierBase(glState)
		, m_prevStencilTestFail(convertToGLenum(stencil_fail))
		, m_prevDepthTestFail(convertToGLenum(depth_fail))
		, m_prevDepthStencilPass(convertToGLenum(depth_stencil_pass))
		, m_stencilTestFail(m_prevStencilTestFail)
		, m_depthTestFail(m_prevDepthTestFail)
		, m_depthStencilPass(m_prevDepthStencilPass)
	{}

	inline static const std::string k_modifierID = "SetStencilOp";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetStencilOpImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevStencilTestFail = parentTypedModifier->m_stencilTestFail;
			m_prevDepthTestFail = parentTypedModifier->m_depthTestFail;
			m_prevDepthStencilPass = parentTypedModifier->m_depthStencilPass;
		}
		else
		{
			glGetIntegerv(GL_STENCIL_FAIL, (GLint*)(&m_prevStencilTestFail));
			glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, (GLint*)(&m_prevDepthTestFail));
			glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, (GLint*)(&m_prevDepthStencilPass));
		}

		glStencilOp(m_stencilTestFail, m_depthTestFail, m_depthStencilPass);
		getStateLog() << "Apply Stencil Op: " << m_stencilTestFail << ", " << m_depthTestFail << ", " << m_depthStencilPass;
	}
	virtual void revert() override
	{
		glStencilOp(m_prevStencilTestFail, m_prevDepthTestFail, m_prevDepthStencilPass);
		getStateLog() << "Revert Stencil Op: " << 
			m_prevStencilTestFail << ", " << m_prevDepthTestFail << ", " << m_prevDepthStencilPass;
	}

	static GLenum convertToGLenum(eGlStencilOp op)
	{
		switch (op)
		{
			case eGlStencilOp::KEEP: return GL_KEEP;
			case eGlStencilOp::ZERO: return GL_ZERO;
			case eGlStencilOp::REPLACE: return GL_REPLACE;
			case eGlStencilOp::INCR: return GL_INCR;
			case eGlStencilOp::INCR_WRAP: return GL_INCR_WRAP;
			case eGlStencilOp::DECR: return GL_DECR;
			case eGlStencilOp::DECR_WRAP: return GL_DECR_WRAP;
			case eGlStencilOp::INVERT: return GL_INVERT;
		}

		return GL_KEEP;
	}

private:
	GLenum m_prevStencilTestFail, m_prevDepthTestFail, m_prevDepthStencilPass;
	GLenum m_stencilTestFail, m_depthTestFail, m_depthStencilPass;
};

void glStateSetStencilOp(GlState& glState, 
						 eGlStencilOp stencil_fail, eGlStencilOp depth_fail, eGlStencilOp depth_stencil_pass)
{
	glState.addModifier(
		std::make_shared<GLStateSetStencilOpImpl>(
			glState, stencil_fail, depth_fail, depth_stencil_pass));
}
// -- GLStateSetBlendEquation --
class GLStateSetBlendEquationImpl : public GLStateModifierBase
{
public:
	GLStateSetBlendEquationImpl() = delete;
	GLStateSetBlendEquationImpl(GlState& glState, const eGlBlendEquation mode)
		: GLStateModifierBase(glState)
		, m_prevMode(convertToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetBlendEquation";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetBlendEquationImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevMode = parentTypedModifier->m_mode;
		}
		else
		{
			glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *)(&m_prevMode));
		}

		glBlendEquation(m_mode);
		getStateLog() << "Apply Blend Eq: " << m_mode;
	}
	virtual void revert() override
	{
		glBlendEquation(m_prevMode);
		getStateLog() << "Revert Blend Eq: " << m_prevMode;
	}

	static GLenum convertToGLenum(eGlBlendEquation mode)
	{
		switch (mode)
		{
		case eGlBlendEquation::ADD: return GL_FUNC_ADD;
		case eGlBlendEquation::SUBTRACT: return GL_FUNC_SUBTRACT;
		case eGlBlendEquation::REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
		case eGlBlendEquation::MIN: return GL_MIN;
		case eGlBlendEquation::MAX: return GL_MAX;
		}

		return GL_FUNC_ADD;
	}

private:
	GLenum m_prevMode;
	GLenum m_mode;
};

void glStateSetBlendEquation(GlState& glState, eGlBlendEquation mode)
{
	glState.addModifier(std::make_shared<GLStateSetBlendEquationImpl>(glState, mode));
}

// -- GLStateSetBlendFunc --
class GLStateSetBlendFuncImpl : public GLStateModifierBase
{
public:
	GLStateSetBlendFuncImpl() = delete;
	GLStateSetBlendFuncImpl(GlState& glState, eGlBlendFunction source_factor, eGlBlendFunction dest_factor)
		: GLStateModifierBase(glState)
		, m_prevSourceFactor(convertToGLenum(source_factor))
		, m_prevDestFactor(convertToGLenum(dest_factor))
		, m_sourceFactor(m_prevSourceFactor)
		, m_destFactor(m_prevDestFactor)
	{}

	inline static const std::string k_modifierID = "SetBlendFunc";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetBlendFuncImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevSourceFactor = parentTypedModifier->m_sourceFactor;
			m_prevDestFactor = parentTypedModifier->m_destFactor;
		}
		else
		{
			glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)(&m_prevSourceFactor));
			glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)(&m_prevDestFactor));
		}

		glBlendFunc(m_sourceFactor, m_destFactor);
		getStateLog() << "Apply Blend Func: " << m_sourceFactor << ", " << m_destFactor;
	}
	virtual void revert() override
	{
		glBlendFunc(m_prevSourceFactor, m_prevDestFactor);
		getStateLog() << "Revert Blend Func: " << m_prevSourceFactor << ", " << m_prevDestFactor;
	}

	static GLenum convertToGLenum(eGlBlendFunction mode)
	{
		switch (mode)
		{
		case eGlBlendFunction::ZERO: return GL_ZERO;
		case eGlBlendFunction::ONE: return GL_ONE;
		case eGlBlendFunction::SRC_COLOR: return GL_SRC_COLOR;
		case eGlBlendFunction::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case eGlBlendFunction::DST_COLOR: return GL_DST_COLOR;
		case eGlBlendFunction::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case eGlBlendFunction::SRC_ALPHA: return GL_SRC_ALPHA;
		case eGlBlendFunction::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case eGlBlendFunction::DST_ALPHA: return GL_DST_ALPHA;
		case eGlBlendFunction::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case eGlBlendFunction::CONSTANT_COLOR: return GL_CONSTANT_COLOR;
		case eGlBlendFunction::ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
		case eGlBlendFunction::CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
		case eGlBlendFunction::ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
		case eGlBlendFunction::SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
		}

		return GL_ONE;
	}

private:
	GLenum m_prevSourceFactor, m_prevDestFactor;
	GLenum m_sourceFactor, m_destFactor;
};

void glStateSetBlendFunc(GlState& glState, eGlBlendFunction source_factor, eGlBlendFunction dest_factor)
{
	glState.addModifier(std::make_shared<GLStateSetBlendFuncImpl>(glState, source_factor, dest_factor));
}

// -- GLStateSetDrawBufferModeFunc --
static GLenum convertGlFrameBufferToGLenum(eGlFrameBuffer mode)
{
	switch (mode)
	{
		case eGlFrameBuffer::FRONT: return GL_FRONT;
		case eGlFrameBuffer::BACK: return GL_BACK;
		case eGlFrameBuffer::LEFT: return GL_LEFT;
		case eGlFrameBuffer::RIGHT: return GL_RIGHT;
		case eGlFrameBuffer::FRONT_LEFT: return GL_FRONT_LEFT;
		case eGlFrameBuffer::FRONT_RIGHT: return GL_FRONT_RIGHT;
		case eGlFrameBuffer::BACK_LEFT: return GL_BACK_LEFT;
		case eGlFrameBuffer::BACK_RIGHT: return GL_BACK_RIGHT;
		case eGlFrameBuffer::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
		case eGlFrameBuffer::NONE: return GL_NONE;
		case eGlFrameBuffer::COLOR_ATTACHMENT0: return GL_COLOR_ATTACHMENT0;
		case eGlFrameBuffer::COLOR_ATTACHMENT1: return GL_COLOR_ATTACHMENT1;
		case eGlFrameBuffer::COLOR_ATTACHMENT2: return GL_COLOR_ATTACHMENT2;
		case eGlFrameBuffer::COLOR_ATTACHMENT3: return GL_COLOR_ATTACHMENT3;
		case eGlFrameBuffer::COLOR_ATTACHMENT4: return GL_COLOR_ATTACHMENT4;
		case eGlFrameBuffer::COLOR_ATTACHMENT5: return GL_COLOR_ATTACHMENT5;
		case eGlFrameBuffer::COLOR_ATTACHMENT6: return GL_COLOR_ATTACHMENT6;
		case eGlFrameBuffer::COLOR_ATTACHMENT7: return GL_COLOR_ATTACHMENT7;
		case eGlFrameBuffer::COLOR_ATTACHMENT8: return GL_COLOR_ATTACHMENT8;
		case eGlFrameBuffer::COLOR_ATTACHMENT9: return GL_COLOR_ATTACHMENT9;
		case eGlFrameBuffer::COLOR_ATTACHMENT10: return GL_COLOR_ATTACHMENT10;
		case eGlFrameBuffer::COLOR_ATTACHMENT11: return GL_COLOR_ATTACHMENT11;
		case eGlFrameBuffer::COLOR_ATTACHMENT12: return GL_COLOR_ATTACHMENT12;
		case eGlFrameBuffer::COLOR_ATTACHMENT13: return GL_COLOR_ATTACHMENT13;
		case eGlFrameBuffer::COLOR_ATTACHMENT14: return GL_COLOR_ATTACHMENT14;
		case eGlFrameBuffer::COLOR_ATTACHMENT15: return GL_COLOR_ATTACHMENT15;
	}

	return GL_ONE;
}

class GLStateSetDrawBufferModeImpl : public GLStateModifierBase
{
public:
	GLStateSetDrawBufferModeImpl() = delete;
	GLStateSetDrawBufferModeImpl(GlState& glState, eGlFrameBuffer mode)
		: GLStateModifierBase(glState)
		, m_prevMode(convertGlFrameBufferToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetDrawBufferMode";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetDrawBufferModeImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevMode = parentTypedModifier->m_mode;
		}
		else
		{
			glGetIntegerv(GL_DRAW_BUFFER, (GLint*)(&m_prevMode));
		}

		glDrawBuffer(m_mode);
		getStateLog() << "Apply Draw Buffer Mode: " << m_mode;
	}
	virtual void revert() override
	{
		glDrawBuffer(m_prevMode);
		getStateLog() << "Revert Draw Buffer Mode: " << m_prevMode;
	}

private:
	GLenum m_prevMode;
	GLenum m_mode;
};

void glStateSetDrawBuffer(GlState& glState, eGlFrameBuffer mode)
{
	glState.addModifier(std::make_shared<GLStateSetDrawBufferModeImpl>(glState, mode));
}

// -- GLStateSetReadBufferMode --
class GLStateSetReadBufferModeImpl : public GLStateModifierBase
{
public:
	GLStateSetReadBufferModeImpl() = delete;
	GLStateSetReadBufferModeImpl(GlState& glState, eGlFrameBuffer mode)
		: GLStateModifierBase(glState)
		, m_prevMode(convertGlFrameBufferToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetReadBufferMode";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) override
	{
		auto parentTypedModifier =
			std::static_pointer_cast<GLStateSetReadBufferModeImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevMode = parentTypedModifier->m_mode;
		}
		else
		{
			glGetIntegerv(GL_READ_BUFFER, (GLint*)(&m_prevMode));
		}

		glReadBuffer(m_mode);
		getStateLog() << "Apply Read Buffer Mode: " << m_mode;
	}
	virtual void revert() override
	{
		glReadBuffer(m_prevMode);
		getStateLog() << "Revert Read Buffer Mode: " << m_prevMode;
	}

private:
	GLenum m_prevMode;
	GLenum m_mode;
};

void glStateSetReadBuffer(GlState& glState, eGlFrameBuffer mode)
{
	glState.addModifier(std::make_shared<GLStateSetReadBufferModeImpl>(glState, mode));
}