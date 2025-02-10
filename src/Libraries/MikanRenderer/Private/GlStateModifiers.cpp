#include "GlCommon.h"
#include "MkStateStack.h"
#include "MkStateLog.h"
#include "MkStateModifiers.h"
#include "IMkState.h"
#include "IMkStateModifier.h"
#include "IMkViewport.h"
#include "IMkWindow.h"

#include "memory"
#include "vector"

// -- GlStateModifierBase --
class GLStateModifierBase : public IMkStateModifier
{
public:
	GLStateModifierBase() = delete;
	GLStateModifierBase(IMkStatePtr mkState)
	: m_ownerGlState(mkState)
	, m_ownerStateStackDepth(mkState->getStackDepth())
	{}

	inline IMkStatePtr getOwnerGlState() { return m_ownerGlState; }
	inline MkStateStack& getOwnerMkStateStack() { return m_ownerGlState->getOwnerStateStack(); }
	inline IMkWindow* getOwnerWindow() { return getOwnerMkStateStack().getOwnerWindow(); }

	inline static const std::string k_modifierID = "<INVALID>";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual int getOwnerStateStackDepth() const override { return m_ownerStateStackDepth; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override {}
	virtual void revert() override {}

	MkStateLog getStateLog()
	{
		return MkStateLog(m_ownerGlState.get());
	}

protected:
	IMkStatePtr m_ownerGlState;
	int m_ownerStateStackDepth;
};

// -- mkStateSetFrontFace --
class GLStateSetFrontFaceImpl : public GLStateModifierBase
{
public:
	GLStateSetFrontFaceImpl() = delete;
	GLStateSetFrontFaceImpl(IMkStatePtr mkState, const eMkFrontFaceMode mode)
		: GLStateModifierBase(mkState)
		, m_prevMode(convertToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetFrontFace";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetFrontFaceImpl>(parentModifier);
		if (parentTypedModifier)
		{
			m_prevMode = parentTypedModifier->m_mode;
		}
		else
		{
			glGetIntegerv(GL_FRONT_FACE, (GLint*)(&m_prevMode));
		}

		glFrontFace(m_mode);
		getStateLog() << "Apply Front Face: " << m_mode;
	}
	virtual void revert() override
	{
		glFrontFace(m_prevMode);
		getStateLog() << "Revert Front Face: " << m_prevMode;
	}

	static GLenum convertToGLenum(eMkFrontFaceMode mode)
	{
		switch (mode)
		{
			case eMkFrontFaceMode::CW: return GL_CW;
			case eMkFrontFaceMode::CCW: return GL_CCW;
		}

		return GL_CW;
	}

private:
	GLenum m_prevMode;
	GLenum m_mode;
};

void mkStateSetFrontFace(IMkStatePtr mkState, eMkFrontFaceMode mode)
{
	mkState->addModifier(std::make_shared<GLStateSetFrontFaceImpl>(mkState, mode));
}

// -- mkStateSetViewport --
class mkStateSetViewportImpl : public GLStateModifierBase
{
public:
	mkStateSetViewportImpl() = delete;
	mkStateSetViewportImpl(IMkStatePtr mkState, int x, int y, int width, int height)
		: GLStateModifierBase(mkState)
		, m_prevX(x), m_prevY(y), m_prevWidth(width), m_prevHeight(height)
		, m_x(x), m_y(y), m_width(width), m_height(height)
	{

	}

	inline static const std::string k_modifierID = "SetViewport";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentViewportModifier = 
			std::static_pointer_cast<const mkStateSetViewportImpl>(parentModifier);
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
		IMkViewportPtr viewport= getOwnerWindow()->getRenderingViewport();
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
		IMkViewportPtr viewport = getOwnerWindow()->getRenderingViewport();
		if (viewport)
		{
			viewport->onRenderingViewportRevert(m_x, m_y, m_width, m_height);
		}
	}

private:
	int32_t m_prevX, m_prevY, m_prevWidth, m_prevHeight;
	int32_t m_x, m_y, m_width, m_height;
};

void mkStateSetViewport(IMkStatePtr mkState, int x, int y, int width, int height)
{
	mkState->addModifier(std::make_shared<mkStateSetViewportImpl>(mkState, x, y, width, height));
}

// -- mkStateSetClearColor --
class mkStateSetClearColorImpl : public GLStateModifierBase
{
public:
	mkStateSetClearColorImpl() = delete;
	mkStateSetClearColorImpl(IMkStatePtr mkState, const glm::vec4& color)
		: GLStateModifierBase(mkState)
		, m_prevClearColor(color)
		, m_clearColor(color)
	{}

	inline static const std::string k_modifierID = "SetBackgroundColor";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentBackgroundColorModifier = 
			std::static_pointer_cast<const mkStateSetClearColorImpl>(parentModifier);
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

void mkStateSetClearColor(IMkStatePtr mkState, const glm::vec4& color)
{
	mkState->addModifier(std::make_shared<mkStateSetClearColorImpl>(mkState, color));
}

// -- mkStateSetColorMask --
class GLStateSetColorMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetColorMaskImpl() = delete;
	GLStateSetColorMaskImpl(IMkStatePtr mkState, const glm::bvec4& colorMask)
		: GLStateModifierBase(mkState)
		, m_prevColorMask(colorMask)
		, m_colorMask(colorMask)
	{}

	inline static const std::string k_modifierID = "SetColorMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentBackgroundColorModifier = 
			std::static_pointer_cast<const GLStateSetColorMaskImpl>(parentModifier);
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

void mkStateSetColorMask(IMkStatePtr mkState, const glm::bvec4& color_mask)
{
	mkState->addModifier(std::make_shared<GLStateSetColorMaskImpl>(mkState, color_mask));
}

// -- mkStateSetDepthMask --
class GLStateSetDepthMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetDepthMaskImpl() = delete;
	GLStateSetDepthMaskImpl(IMkStatePtr mkState, bool depthMask)
		: GLStateModifierBase(mkState)
		, m_prevDepthMask(depthMask)
		, m_depthMask(depthMask)
	{}

	inline static const std::string k_modifierID = "SetDepthMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier = 
			std::static_pointer_cast<const GLStateSetDepthMaskImpl>(parentModifier);
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

void mkStateSetDepthMask(IMkStatePtr mkState, bool depth_mask)
{
	mkState->addModifier(std::make_shared<GLStateSetDepthMaskImpl>(mkState, depth_mask));
}

// -- mkStateSetStencilBufferClearValue --
class GLStateSetStencilBufferClearValueImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilBufferClearValueImpl() = delete;
	GLStateSetStencilBufferClearValueImpl(IMkStatePtr mkState, const int value)
		: GLStateModifierBase(mkState)
		, m_prevValue(value)
		, m_value(value)
	{}

	inline static const std::string k_modifierID = "SetStencilBufferClearValue";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier = 
			std::static_pointer_cast<const GLStateSetStencilBufferClearValueImpl>(parentModifier);
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

void mkStateSetStencilBufferClearValue(IMkStatePtr mkState, int value)
{
	mkState->addModifier(std::make_shared<GLStateSetStencilBufferClearValueImpl>(mkState, value));
}

// -- GLStateSetStencilMaskImpl --
class GLStateSetStencilMaskImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilMaskImpl() = delete;
	GLStateSetStencilMaskImpl(IMkStatePtr mkState, const uint32_t mask)
		: GLStateModifierBase(mkState)
		, m_prevMask(mask)
		, m_mask(mask)
	{}

	inline static const std::string k_modifierID = "SetStencilMask";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetStencilMaskImpl>(parentModifier);
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

void mkStateSetStencilMask(IMkStatePtr mkState, uint32_t mask)
{
	mkState->addModifier(std::make_shared<GLStateSetStencilMaskImpl>(mkState, mask));
}

// -- mkStateSetStencilFunc --
class GLStateSetStencilFuncImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilFuncImpl() = delete;
	GLStateSetStencilFuncImpl(IMkStatePtr mkState, eMkStencilFunction func, int ref, uint32_t mask)
		: GLStateModifierBase(mkState)
		, m_prevFunc(convertToGLenum(func))
		, m_func(m_prevFunc)
		, m_prevRef(ref)
		, m_ref(ref)
		, m_prevMask(mask)
		, m_mask(mask)
	{}

	inline static const std::string k_modifierID = "SetStencilFunc";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetStencilFuncImpl>(parentModifier);
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

	static GLenum convertToGLenum(eMkStencilFunction func)
	{
		switch (func)
		{
		case eMkStencilFunction::NEVER: return GL_NEVER;
		case eMkStencilFunction::LESS: return GL_LESS;
		case eMkStencilFunction::LEQUAL: return GL_LEQUAL;
		case eMkStencilFunction::GREATER: return GL_GREATER;
		case eMkStencilFunction::GEQUAL: return GL_GEQUAL;
		case eMkStencilFunction::EQUAL: return GL_EQUAL;
		case eMkStencilFunction::NOTEQUAL: return GL_NOTEQUAL;
		case eMkStencilFunction::ALWAYS: return GL_ALWAYS;
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

void mkStateSetStencilFunc(IMkStatePtr mkState, eMkStencilFunction func, int ref, uint32_t mask)
{
	mkState->addModifier(std::make_shared<GLStateSetStencilFuncImpl>(mkState, func, ref, mask));
}

// -- mkStateSetStencilOp --
class GLStateSetStencilOpImpl : public GLStateModifierBase
{
public:
	GLStateSetStencilOpImpl() = delete;
	GLStateSetStencilOpImpl(
		IMkStatePtr mkState, 
		eMkStencilOp stencil_fail, 
		eMkStencilOp depth_fail, 
		eMkStencilOp depth_stencil_pass)
		: GLStateModifierBase(mkState)
		, m_prevStencilTestFail(convertToGLenum(stencil_fail))
		, m_prevDepthTestFail(convertToGLenum(depth_fail))
		, m_prevDepthStencilPass(convertToGLenum(depth_stencil_pass))
		, m_stencilTestFail(m_prevStencilTestFail)
		, m_depthTestFail(m_prevDepthTestFail)
		, m_depthStencilPass(m_prevDepthStencilPass)
	{}

	inline static const std::string k_modifierID = "SetStencilOp";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetStencilOpImpl>(parentModifier);
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

	static GLenum convertToGLenum(eMkStencilOp op)
	{
		switch (op)
		{
			case eMkStencilOp::KEEP: return GL_KEEP;
			case eMkStencilOp::ZERO: return GL_ZERO;
			case eMkStencilOp::REPLACE: return GL_REPLACE;
			case eMkStencilOp::INCR: return GL_INCR;
			case eMkStencilOp::INCR_WRAP: return GL_INCR_WRAP;
			case eMkStencilOp::DECR: return GL_DECR;
			case eMkStencilOp::DECR_WRAP: return GL_DECR_WRAP;
			case eMkStencilOp::INVERT: return GL_INVERT;
		}

		return GL_KEEP;
	}

private:
	GLenum m_prevStencilTestFail, m_prevDepthTestFail, m_prevDepthStencilPass;
	GLenum m_stencilTestFail, m_depthTestFail, m_depthStencilPass;
};

void mkStateSetStencilOp(IMkStatePtr mkState, 
						 eMkStencilOp stencil_fail, eMkStencilOp depth_fail, eMkStencilOp depth_stencil_pass)
{
	mkState->addModifier(
		std::make_shared<GLStateSetStencilOpImpl>(
			mkState, stencil_fail, depth_fail, depth_stencil_pass));
}

// -- mkStateSetBlendEquation --
class GLStateSetBlendEquationImpl : public GLStateModifierBase
{
public:
	GLStateSetBlendEquationImpl() = delete;
	GLStateSetBlendEquationImpl(IMkStatePtr mkState, const eMkBlendEquation mode)
		: GLStateModifierBase(mkState)
		, m_prevMode(convertToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetBlendEquation";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetBlendEquationImpl>(parentModifier);
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

	static GLenum convertToGLenum(eMkBlendEquation mode)
	{
		switch (mode)
		{
		case eMkBlendEquation::ADD: return GL_FUNC_ADD;
		case eMkBlendEquation::SUBTRACT: return GL_FUNC_SUBTRACT;
		case eMkBlendEquation::REVERSE_SUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
		case eMkBlendEquation::MIN: return GL_MIN;
		case eMkBlendEquation::MAX: return GL_MAX;
		}

		return GL_FUNC_ADD;
	}

private:
	GLenum m_prevMode;
	GLenum m_mode;
};

void mkStateSetBlendEquation(IMkStatePtr mkState, eMkBlendEquation mode)
{
	mkState->addModifier(std::make_shared<GLStateSetBlendEquationImpl>(mkState, mode));
}

// -- mkStateSetBlendFunc --
class GLStateSetBlendFuncImpl : public GLStateModifierBase
{
public:
	GLStateSetBlendFuncImpl() = delete;
	GLStateSetBlendFuncImpl(IMkStatePtr mkState, eMkBlendFunction source_factor, eMkBlendFunction dest_factor)
		: GLStateModifierBase(mkState)
		, m_prevSourceFactor(convertToGLenum(source_factor))
		, m_prevDestFactor(convertToGLenum(dest_factor))
		, m_sourceFactor(m_prevSourceFactor)
		, m_destFactor(m_prevDestFactor)
	{}

	inline static const std::string k_modifierID = "SetBlendFunc";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetBlendFuncImpl>(parentModifier);
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

	static GLenum convertToGLenum(eMkBlendFunction mode)
	{
		switch (mode)
		{
		case eMkBlendFunction::ZERO: return GL_ZERO;
		case eMkBlendFunction::ONE: return GL_ONE;
		case eMkBlendFunction::SRC_COLOR: return GL_SRC_COLOR;
		case eMkBlendFunction::ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case eMkBlendFunction::DST_COLOR: return GL_DST_COLOR;
		case eMkBlendFunction::ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case eMkBlendFunction::SRC_ALPHA: return GL_SRC_ALPHA;
		case eMkBlendFunction::ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case eMkBlendFunction::DST_ALPHA: return GL_DST_ALPHA;
		case eMkBlendFunction::ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case eMkBlendFunction::CONSTANT_COLOR: return GL_CONSTANT_COLOR;
		case eMkBlendFunction::ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
		case eMkBlendFunction::CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
		case eMkBlendFunction::ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
		case eMkBlendFunction::SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
		}

		return GL_ONE;
	}

private:
	GLenum m_prevSourceFactor, m_prevDestFactor;
	GLenum m_sourceFactor, m_destFactor;
};

void mkStateSetBlendFunc(IMkStatePtr mkState, eMkBlendFunction source_factor, eMkBlendFunction dest_factor)
{
	mkState->addModifier(std::make_shared<GLStateSetBlendFuncImpl>(mkState, source_factor, dest_factor));
}

// -- GLStateSetDrawBufferModeFunc --
static GLenum convertGlFrameBufferToGLenum(eMkFrameBuffer mode)
{
	switch (mode)
	{
		case eMkFrameBuffer::FRONT: return GL_FRONT;
		case eMkFrameBuffer::BACK: return GL_BACK;
		case eMkFrameBuffer::LEFT: return GL_LEFT;
		case eMkFrameBuffer::RIGHT: return GL_RIGHT;
		case eMkFrameBuffer::FRONT_LEFT: return GL_FRONT_LEFT;
		case eMkFrameBuffer::FRONT_RIGHT: return GL_FRONT_RIGHT;
		case eMkFrameBuffer::BACK_LEFT: return GL_BACK_LEFT;
		case eMkFrameBuffer::BACK_RIGHT: return GL_BACK_RIGHT;
		case eMkFrameBuffer::FRONT_AND_BACK: return GL_FRONT_AND_BACK;
		case eMkFrameBuffer::NONE: return GL_NONE;
		case eMkFrameBuffer::COLOR_ATTACHMENT0: return GL_COLOR_ATTACHMENT0;
		case eMkFrameBuffer::COLOR_ATTACHMENT1: return GL_COLOR_ATTACHMENT1;
		case eMkFrameBuffer::COLOR_ATTACHMENT2: return GL_COLOR_ATTACHMENT2;
		case eMkFrameBuffer::COLOR_ATTACHMENT3: return GL_COLOR_ATTACHMENT3;
		case eMkFrameBuffer::COLOR_ATTACHMENT4: return GL_COLOR_ATTACHMENT4;
		case eMkFrameBuffer::COLOR_ATTACHMENT5: return GL_COLOR_ATTACHMENT5;
		case eMkFrameBuffer::COLOR_ATTACHMENT6: return GL_COLOR_ATTACHMENT6;
		case eMkFrameBuffer::COLOR_ATTACHMENT7: return GL_COLOR_ATTACHMENT7;
		case eMkFrameBuffer::COLOR_ATTACHMENT8: return GL_COLOR_ATTACHMENT8;
		case eMkFrameBuffer::COLOR_ATTACHMENT9: return GL_COLOR_ATTACHMENT9;
		case eMkFrameBuffer::COLOR_ATTACHMENT10: return GL_COLOR_ATTACHMENT10;
		case eMkFrameBuffer::COLOR_ATTACHMENT11: return GL_COLOR_ATTACHMENT11;
		case eMkFrameBuffer::COLOR_ATTACHMENT12: return GL_COLOR_ATTACHMENT12;
		case eMkFrameBuffer::COLOR_ATTACHMENT13: return GL_COLOR_ATTACHMENT13;
		case eMkFrameBuffer::COLOR_ATTACHMENT14: return GL_COLOR_ATTACHMENT14;
		case eMkFrameBuffer::COLOR_ATTACHMENT15: return GL_COLOR_ATTACHMENT15;
	}

	return GL_ONE;
}

class GLStateSetDrawBufferModeImpl : public GLStateModifierBase
{
public:
	GLStateSetDrawBufferModeImpl() = delete;
	GLStateSetDrawBufferModeImpl(IMkStatePtr mkState, eMkFrameBuffer mode)
		: GLStateModifierBase(mkState)
		, m_prevMode(convertGlFrameBufferToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetDrawBufferMode";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetDrawBufferModeImpl>(parentModifier);
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

void mkStateSetDrawBuffer(IMkStatePtr mkState, eMkFrameBuffer mode)
{
	mkState->addModifier(std::make_shared<GLStateSetDrawBufferModeImpl>(mkState, mode));
}

// -- GLStateSetReadBufferMode --
class GLStateSetReadBufferModeImpl : public GLStateModifierBase
{
public:
	GLStateSetReadBufferModeImpl() = delete;
	GLStateSetReadBufferModeImpl(IMkStatePtr mkState, eMkFrameBuffer mode)
		: GLStateModifierBase(mkState)
		, m_prevMode(convertGlFrameBufferToGLenum(mode))
		, m_mode(m_prevMode)
	{}

	inline static const std::string k_modifierID = "SetReadBufferMode";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		const auto parentTypedModifier =
			std::static_pointer_cast<const GLStateSetReadBufferModeImpl>(parentModifier);
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

void mkStateSetReadBuffer(IMkStatePtr mkState, eMkFrameBuffer mode)
{
	mkState->addModifier(std::make_shared<GLStateSetReadBufferModeImpl>(mkState, mode));
}

// -- mkStateClearBuffer --
class GLStateClearBufferImpl : public GLStateModifierBase
{
public:
	GLStateClearBufferImpl() = delete;
	GLStateClearBufferImpl(IMkStatePtr mkState, const eMkClearFlags flags)
		: GLStateModifierBase(mkState)
		, m_flags(flags)
	{
	}

	inline static const std::string k_modifierID = "ClearBuffer";
	virtual const std::string& getModifierID() const override { return k_modifierID; }
	virtual void apply(IMkStateModifierConstPtr parentModifier) override
	{
		GLbitfield bitfield = 0;

		if (has_any_bits_set(m_flags & eMkClearFlags::color))
		{
			bitfield |= GL_COLOR_BUFFER_BIT;
			getStateLog() << "Clear Color Buffer";
		}

		if (has_any_bits_set(m_flags & eMkClearFlags::depth))
		{
			bitfield |= GL_DEPTH_BUFFER_BIT;
			getStateLog() << "Clear Depth Buffer";
		}

		if (has_any_bits_set(m_flags & eMkClearFlags::stencil))
		{
			bitfield |= GL_STENCIL_BUFFER_BIT;
			getStateLog() << "Clear Stencil Buffer";
		}

		if (bitfield != 0)
		{
			glClear(bitfield);
		}
	}

private:
	eMkClearFlags m_flags;
};

void mkStateClearBuffer(IMkStatePtr mkState, eMkClearFlags flags)
{
	mkState->addModifier(std::make_shared<GLStateClearBufferImpl>(mkState, flags));
}