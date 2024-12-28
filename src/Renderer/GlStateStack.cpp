#include "GlStateStack.h"
#include "GlCommon.h"
#include "IGlWindow.h"
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
GlState::GlState(GlStateStack& ownerStack, const int stackDepth)
	: m_ownerStack(ownerStack)
	, m_parentState(ownerStack.getState(stackDepth - 1))
	, m_stackDepth(stackDepth)
{
	if (m_parentState != nullptr)
	{
		// init our stack state with a copy of our parent state
		memcpy(m_flags, m_parentState->m_flags, sizeof(m_flags));

		// Copy all the modifiers from the parent state
		m_modifiers = m_parentState->m_modifiers;
	}
	else
	{
		// Fetch the initial state of all the flags
		for (int flagIndex = 0; flagIndex < (int)eGlStateFlagType::COUNT; ++flagIndex)
		{
			const GLenum glFlag = g_glFlagTypeMapping[flagIndex];

			m_flags[flagIndex] = (glIsEnabled(glFlag) == GL_TRUE);
		}
	}
}

GlState::~GlState()
{
	// Restore to the parent flags, if there is a parent state
	if (m_parentState != nullptr)
	{
		for (int flagIndex = 0; flagIndex < (int)eGlStateFlagType::COUNT; ++flagIndex)
		{
			const GLenum glFlag = g_glFlagTypeMapping[flagIndex];
			const bool parentFlagValue = m_parentState->m_flags[flagIndex];

			if (parentFlagValue)
			{
				glEnable(glFlag);
			}
			else
			{
				glDisable(glFlag);
			}
		}
	}

	// Revert the effect of the modifiers applied in this state
	for (auto modifierIt = m_modifiers.begin(); modifierIt != m_modifiers.end(); ++modifierIt)
	{
		GlStateModifierPtr modifer= modifierIt->second;

		// If this modifier was first created in this state, revert it
		if (modifer->getOwnerStateStackDepth() == m_stackDepth)
		{
			modifierIt->second->revert();
		}
	}
}

GlState& GlState::enableFlag(eGlStateFlagType flagType)
{
	glEnable(g_glFlagTypeMapping[(int)flagType]);
	m_flags[(int)flagType]= true;

	return *this;
}

GlState& GlState::disableFlag(eGlStateFlagType flagType)
{
	glDisable(g_glFlagTypeMapping[(int)flagType]);
	m_flags[(int)flagType] = false;

	return *this;
}

GlState& GlState::addModifier(GlStateModifierPtr modifier)
{
	if (modifier)
	{
		auto existingModifierIt= m_modifiers.find(modifier->getModifierID());
		if (existingModifierIt != m_modifiers.end())
		{
			GlStateModifierPtr parentModifier= existingModifierIt->second;
			modifier->apply(parentModifier);
		}
		else
		{
			modifier->apply(GlStateModifierPtr());
		}

		m_modifiers[modifier->getModifierID()]= modifier;
	}

	return *this;
}

// -- GlScopedState -----
GlScopedState::GlScopedState(const std::string& scopeName, GlState& state) 
	: m_scopeName(scopeName)
	, m_state(state)
{
	if (!m_scopeName.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, m_scopeName.c_str());
	}
}

GlScopedState::~GlScopedState()
{
	// Make sure we are deleting the state on the top of the stack
	assert(m_state.getOwnerStateStack().getCurrentStackDepth() == m_state.getStackDepth());
	m_state.getOwnerStateStack().popState();

	if (!m_scopeName.empty())
	{
		glPopDebugGroup();
	}
}

// -- GlStateStack -----
GlStateStack::GlStateStack(IGlWindow* ownerWindow) 
	: m_ownerWindow(ownerWindow)
{
}

GlStateStack::~GlStateStack()
{
	while (!m_stateStack.empty())
	{
		popState();
	}
}

GlState& GlStateStack::pushState()
{
	// Create a new GlState and initialize it from the parent state on this stack
	GlState* state = new GlState(*this, (int)m_stateStack.size());

	// Add it to the top of the stack
	m_stateStack.push_back(state);

	return *state;
}

int GlStateStack::getCurrentStackDepth() const
{
	return (int)m_stateStack.size() - 1;
}

GlState* GlStateStack::getState(const int depth) const
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

GlScopedState GlStateStack::createScopedState(const std::string& scopeName)
{
	// Create a state that will get auto cleaned up when GLScopedState goes out of scope
	return GlScopedState(scopeName, pushState());
}