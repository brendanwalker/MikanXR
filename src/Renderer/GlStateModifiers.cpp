#include "GlCommon.h"
#include "GlStateStack.h"
#include "IGlStateModifier.h"
#include "GlStateModifiers.h"

#include "memory"
#include "vector"

// -- GlStateSetViewport --
class GLStateSetViewportImpl : public IGlStateModifier
{
public:
	GLStateSetViewportImpl() = delete;
	GLStateSetViewportImpl(int x, int y, int width, int height)
		: m_prevX(x), m_prevY(y), m_prevWidth(width), m_prevHeight(height)
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
	}
	virtual void revert() override
	{
		glViewport(m_prevX, m_prevY, m_prevWidth, m_prevHeight);
	}

private:
	int32_t m_prevX, m_prevY, m_prevWidth, m_prevHeight;
	int32_t m_x, m_y, m_width, m_height;
};

void glStateSetViewport(GlState& glState, int x, int y, int width, int height)
{
	glState.addModifier(std::make_shared<GLStateSetViewportImpl>(x, y, width, height));
}

// -- GLStateSetClearColor --
class GLStateSetClearColorImpl : public IGlStateModifier
{
public:
	GLStateSetClearColorImpl() = delete;
	GLStateSetClearColorImpl(const glm::vec4& color)
		: m_prevClearColor(color)
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
	}
	virtual void revert() override
	{
		glClearColor(m_prevClearColor.r, m_prevClearColor.g, m_prevClearColor.b, m_prevClearColor.a);
	}

private:
	glm::vec4 m_prevClearColor;
	glm::vec4 m_clearColor;
};

void glStateSetClearColor(GlState& glState, const glm::vec4& color)
{
	glState.addModifier(std::make_shared<GLStateSetClearColorImpl>(color));
}

// -- GLStateSetStencilBufferClearValue --
// -- GLStateSetStencilFunc --
// -- GLStateSetStencilOp --
// -- GLStateSetBlendEquation --
// -- GLStateSetBlendFunc --