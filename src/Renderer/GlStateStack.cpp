#include "GlStateStack.h"
#include "GlCommon.h"
#include <assert.h>

GLenum g_glFlagTypeMapping[(int)eGlStateFlagType::COUNT] = {
	GL_LIGHT0,				// light0,
	GL_TEXTURE_2D,			// texture2d,
	GL_DEPTH_TEST,			// depthTest,
	GL_STENCIL_TEST,		// stencilTest
	GL_SCISSOR_TEST,		// scissorTest
	GL_BLEND,				// blend,
	GL_CULL_FACE,			// cullFace,
	GL_PROGRAM_POINT_SIZE,	// programPointSize,
};

// -- GLScopedState -----
GLState::GLState(GlStateStack& ownerStack, const int stackDepth)
	: m_ownerStack(ownerStack)
	, m_parentState(ownerStack.getState(stackDepth - 1))
	, m_stackDepth(stackDepth)
{
	if (m_parentState != nullptr)
	{
		// init our stack state with a copy of our parent state
		memcpy(m_flags, m_parentState->m_flags, sizeof(m_flags));
	}
	else
	{
		// Initialize all stack vars with defaults
		memset(&m_flags, 0, sizeof(m_flags));
	}
}

GLState::~GLState()
{
	for (int flagIndex = 0; flagIndex < (int)eGlStateFlagType::COUNT; ++flagIndex)
	{
		const eGlStateFlagValue flagValue = m_flags[flagIndex];
		const eGlStateFlagValue parentFlagValue= 
			(m_parentState != nullptr) 
			? m_parentState->m_flags[flagIndex] 
			: eGlStateFlagValue::unset;

		if (flagValue != parentFlagValue)
		{
			GLenum glFlag= g_glFlagTypeMapping[flagIndex];

			if (flagValue == eGlStateFlagValue::enabled)
			{
				glDisable(glFlag);
			}
			else if (flagValue == eGlStateFlagValue::disabled)
			{
				glEnable(glFlag);
			}
		}
	}
}

GLState& GLState::enableFlag(eGlStateFlagType flagType)
{
	const eGlStateFlagValue flagValue= m_flags[(int)flagType];
	if (flagValue != eGlStateFlagValue::enabled)
	{
		glEnable(g_glFlagTypeMapping[(int)flagType]);
		m_flags[(int)flagType]= eGlStateFlagValue::enabled;
	}

	return *this;
}

GLState& GLState::disableFlag(eGlStateFlagType flagType)
{
	const eGlStateFlagValue flagValue = m_flags[(int)flagType];
	if (flagValue != eGlStateFlagValue::disabled)
	{
		glDisable(g_glFlagTypeMapping[(int)flagType]);
		m_flags[(int)flagType] = eGlStateFlagValue::disabled;
	}

	return *this;
}

// -- GlScopedState -----
GLScopedState::GLScopedState(GLState& state) : m_state(state)
{
}

GLScopedState::~GLScopedState()
{
	// Make sure we are deleting the state on the top of the stack
	assert(m_state.getOwnerStateStack().getCurrentStackDepth() == m_state.getStackDepth());
	m_state.getOwnerStateStack().popState();
}

// -- GlStateStack -----
GlStateStack::~GlStateStack()
{
	while (!m_stateStack.empty())
	{
		popState();
	}
}

GLState& GlStateStack::pushState()
{
	// Create a new GlState and initialize it from the parent state on this stack
	GLState* state = new GLState(*this, (int)m_stateStack.size());

	// Add it to the top of the stack
	m_stateStack.push_back(state);

	return *state;
}

int GlStateStack::getCurrentStackDepth() const
{
	return (int)m_stateStack.size() - 1;
}

GLState* GlStateStack::getState(const int depth) const
{
	return (depth >= 0 && depth < (int)m_stateStack.size()) ? m_stateStack[depth] : nullptr;
}

void GlStateStack::popState()
{
	const int currentDepth = getCurrentStackDepth();

	if (currentDepth >= 0)
	{
		// Cleaning up the state will undo all the flags it set
		delete m_stateStack[currentDepth];

		// Remove the state from the top of the stack
		m_stateStack.pop_back();
	}
}

GLScopedState GlStateStack::createScopedState()
{
	// Create a state that will get auto cleaned up when GLScopedState goes out of scope
	return GLScopedState(pushState());
}