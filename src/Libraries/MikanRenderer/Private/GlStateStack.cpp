#include "GlStateStack.h"
#include "GlCommon.h"
#include "IGlWindow.h"
#include <assert.h>

#define GL_STATE_DEBUG_PRINT_TAB	"  "

const char* g_glFlagName[(int)eGlStateFlagType::COUNT] = {
	"GL_LIGHT0",				// light0,
	"GL_TEXTURE_2D",			// texture2d,
	"GL_DEPTH_TEST",			// depthTest,
	"GL_STENCIL_TEST",			// stencilTest
	"GL_SCISSOR_TEST",			// scissorTest
	"GL_BLEND",					// blend,
	"GL_CULL_FACE",				// cullFace,
	"GL_PROGRAM_POINT_SIZE",	// programPointSize,
};

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
GlState::GlState(GlStateStack& ownerStack, const std::string& scopeName, const int stackDepth)
	: m_ownerStack(ownerStack)
	, m_parentState(ownerStack.getState(stackDepth - 1))
	, m_scopeName(scopeName)
	, m_stackDepth(stackDepth)
	, m_debugPrefix(m_parentState ? m_parentState->getDebugPrefix() : "")
{
	// Log that we are pushing the state
	GlStateLog(*this) << "  ";
	if (!m_scopeName.empty())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, m_scopeName.c_str());
		GlStateLog(*this) << "Pushed state scope: " << m_scopeName;
	}
	else
	{
		GlStateLog(*this) << "Pushed state scope: <No Name>";
	}

	// Add tab to the parent debug prefix to indent the debug output inside this state
	m_debugPrefix += GL_STATE_DEBUG_PRINT_TAB; 

	// Initialize the state flags
	if (m_parentState != nullptr)
	{
		// init our stack state with a copy of our parent state
		memcpy(m_flags, m_parentState->m_flags, sizeof(m_flags));
	}
	else
	{
		// Fetch the initial state of all the flags
		for (int flagIndex = 0; flagIndex < (int)eGlStateFlagType::COUNT; ++flagIndex)
		{
			const GLenum glFlag = g_glFlagTypeMapping[flagIndex];
			const bool isFlagEnabled= (glIsEnabled(glFlag) == GL_TRUE);

			m_flags[flagIndex] = isFlagEnabled;
			GlStateLog(*this) << "Initial Flag: " << g_glFlagName[flagIndex] << "=" << isFlagEnabled;
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
			const bool currentflagValue= m_flags[flagIndex];
			const bool parentFlagValue = m_parentState->m_flags[flagIndex];

			if (parentFlagValue != currentflagValue)
			{
				if (parentFlagValue)
				{
					glEnable(glFlag);
				}
				else
				{
					glDisable(glFlag);
				}

				GlStateLog(*this) << "Restore Flag: " << g_glFlagName[flagIndex] << " " 
					<< currentflagValue << " -> " << parentFlagValue;
			}
		}
	}

	// Revert the effect of the modifiers applied in this state
	for (auto modifierIt = m_modifiers.begin(); modifierIt != m_modifiers.end(); ++modifierIt)
	{
		GlStateModifierPtr modifer= modifierIt->second;

		// Revert the modifier
		assert(modifer->getOwnerStateStackDepth() == m_stackDepth);
		modifer->revert();
	}

	// Reset debug prefix back to the parent indent before the final 'pop' debug output
	if (m_parentState)
	{
		m_debugPrefix = m_parentState->getDebugPrefix();
	}
	else
	{
		m_debugPrefix = "";
	}

	// Log that we are popping the state
	if (!m_scopeName.empty())
	{
		glPopDebugGroup();
		GlStateLog(*this) << "Popped state scope: " << m_scopeName;
	}
	else
	{
		GlStateLog(*this) << "Popped state scope: <No Name>";
	}
	GlStateLog(*this) << "  ";
}

GlState& GlState::enableFlag(eGlStateFlagType flagType)
{
	int flagIndex= (int)flagType;

	if (!m_flags[flagIndex])
	{
		glEnable(g_glFlagTypeMapping[flagIndex]);
		m_flags[flagIndex] = true;

		GlStateLog(*this) << "Enable Flag: " << g_glFlagName[flagIndex];
	}

	return *this;
}

GlState& GlState::disableFlag(eGlStateFlagType flagType)
{
	int flagIndex = (int)flagType;

	if (m_flags[flagIndex])
	{
		glDisable(g_glFlagTypeMapping[flagIndex]);
		m_flags[flagIndex] = false;

		GlStateLog(*this) << "Disable Flag: " << g_glFlagName[flagIndex];
	}

	return *this;
}

bool GlState::isFlagEnabled(eGlStateFlagType flagType) const
{
	return m_flags[(int)flagType];
}

GlStateModifierPtr GlState::findParentModifier(GlStateModifierPtr modifier) const
{
	if (modifier && m_parentState)
	{
		// See if out parent state has a modifier with the same ID
		auto existingModifierIt = m_parentState->m_modifiers.find(modifier->getModifierID());
		if (existingModifierIt != m_parentState->m_modifiers.end())
		{
			return existingModifierIt->second;
		}

		// Recurse into the parent state to continue the search from there
		return m_parentState->findParentModifier(modifier);
	}

	return GlStateModifierPtr();
}

GlState& GlState::addModifier(GlStateModifierPtr modifier)
{
	if (modifier)
	{
		// Revert any existing modifier in this state
		auto existingModifierIt= m_modifiers.find(modifier->getModifierID());
		if (existingModifierIt != m_modifiers.end())
		{
			MIKAN_LOG_WARNING("addModifier") 
				<< "Redundant modifier of ID: " << modifier->getModifierID() 
				<< " in same scope: " << m_scopeName;

			//TODO: Add an efficiency warning that we should really make a new GlStateScope
			// so that we aren't needlessly stomp on the existing modifier in the same scope
			existingModifierIt->second->revert();

			// Deallocate the existing modifier
			m_modifiers[modifier->getModifierID()] = nullptr;
		}

		// See if a modifier of the same type is applied in any parent state
		GlStateModifierPtr parentModifier = findParentModifier(modifier);

		// Apply the new modifier
		modifier->apply(parentModifier);

		// Assign the modifier to this state
		m_modifiers[modifier->getModifierID()]= modifier;
	}

	return *this;
}

// -- GlScopedState -----
GlScopedState::GlScopedState(GlState& state) 
	: m_state(state)
{
}

GlScopedState::~GlScopedState()
{
	// Make sure we are deleting the state on the top of the stack
	assert(m_state.getOwnerStateStack().getCurrentStackDepth() == m_state.getStackDepth());

	// Pop the state last since this will invalidate the state reference
	m_state.getOwnerStateStack().popState();
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

GlState& GlStateStack::pushState(const std::string& scopeName)
{
	// Create a new GlState and initialize it from the parent state on this stack
	GlState* state = new GlState(*this, scopeName, (int)m_stateStack.size());

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
	return GlScopedState(pushState(scopeName));
}