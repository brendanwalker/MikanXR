#include "IMkState.h"
#include "IMkStateModifier.h"
#include "MkStateStack.h"
#include "MkStateLog.h"
#include "GlCommon.h"

#include <map>
#include <assert.h>

#define GL_STATE_DEBUG_PRINT_TAB	"  "

const char* g_glFlagName[(int)eMkStateFlagType::COUNT] = {
	"GL_LIGHT0",				// light0,
	"GL_TEXTURE_2D",			// texture2d,
	"GL_DEPTH_TEST",			// depthTest,
	"GL_STENCIL_TEST",			// stencilTest
	"GL_SCISSOR_TEST",			// scissorTest
	"GL_BLEND",					// blend,
	"GL_CULL_FACE",				// cullFace,
	"GL_PROGRAM_POINT_SIZE",	// programPointSize,
};

GLenum g_glFlagTypeMapping[(int)eMkStateFlagType::COUNT] = {
	GL_LIGHT0,				// light0,
	GL_TEXTURE_2D,			// texture2d,
	GL_DEPTH_TEST,			// depthTest,
	GL_STENCIL_TEST,		// stencilTest
	GL_SCISSOR_TEST,		// scissorTest
	GL_BLEND,				// blend,
	GL_CULL_FACE,			// cullFace,
	GL_PROGRAM_POINT_SIZE,	// programPointSize,
};

class GlState : public IMkState
{
public:
	GlState::GlState(MkStateStack& ownerStack, const std::string& scopeName, const int stackDepth)
		: m_ownerStack(ownerStack)
		, m_parentState(ownerStack.getState(stackDepth - 1))
		, m_scopeName(scopeName)
		, m_stackDepth(stackDepth)
		, m_debugPrefix(m_parentState ? m_parentState->getDebugPrefix() : "")
	{
		// Log that we are pushing the state
		MkStateLog(this) << "  ";
		if (!m_scopeName.empty())
		{
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, m_scopeName.c_str());
			MkStateLog(this) << "Pushed state scope: " << m_scopeName;
		}
		else
		{
			MkStateLog(this) << "Pushed state scope: <No Name>";
		}

		// Add tab to the parent debug prefix to indent the debug output inside this state
		m_debugPrefix += GL_STATE_DEBUG_PRINT_TAB;

		// Initialize the state flags
		if (m_parentState != nullptr)
		{
			// init our stack state with a copy of our parent state
			const GlState* parentGlState= static_cast<const GlState*>(m_parentState.get());
			memcpy(m_flags, parentGlState->m_flags, sizeof(m_flags));
		}
		else
		{
			// Fetch the initial state of all the flags
			for (int flagIndex = 0; flagIndex < (int)eMkStateFlagType::COUNT; ++flagIndex)
			{
				const GLenum glFlag = g_glFlagTypeMapping[flagIndex];
				const bool isFlagEnabled = (glIsEnabled(glFlag) == GL_TRUE);

				m_flags[flagIndex] = isFlagEnabled;
				MkStateLog(this) << "Initial Flag: " << g_glFlagName[flagIndex] << "=" << isFlagEnabled;
			}
		}
	}

	GlState::~GlState()
	{
		// Restore to the parent flags, if there is a parent state
		if (m_parentState != nullptr)
		{
			const GlState* parentGlState= static_cast<const GlState*>(m_parentState.get());

			for (int flagIndex = 0; flagIndex < (int)eMkStateFlagType::COUNT; ++flagIndex)
			{
				const GLenum glFlag = g_glFlagTypeMapping[flagIndex];
				const bool currentflagValue = m_flags[flagIndex];
				const bool parentFlagValue = parentGlState->m_flags[flagIndex];

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

					MkStateLog(this) << "Restore Flag: " << g_glFlagName[flagIndex] << " "
						<< currentflagValue << " -> " << parentFlagValue;
				}
			}
		}

		// Revert the effect of the modifiers applied in this state
		for (auto modifierIt = m_modifiers.begin(); modifierIt != m_modifiers.end(); ++modifierIt)
		{
			IMkStateModifierPtr modifer = modifierIt->second;

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
			MkStateLog(this) << "Popped state scope: " << m_scopeName;
		}
		else
		{
			MkStateLog(this) << "Popped state scope: <No Name>";
		}
		MkStateLog(this) << "  ";
	}

	virtual MkStateStack& getOwnerStateStack() const override
	{ 
		return m_ownerStack; 
	}

	virtual int getStackDepth() const override
	{ 
		return m_stackDepth;
	}

	virtual const std::string& getDebugPrefix() const override
	{ 
		return m_debugPrefix;
	}

	virtual IMkState* enableFlag(eMkStateFlagType flagType) override
	{
		int flagIndex = (int)flagType;

		if (!m_flags[flagIndex])
		{
			glEnable(g_glFlagTypeMapping[flagIndex]);
			m_flags[flagIndex] = true;

			MkStateLog(this) << "Enable Flag: " << g_glFlagName[flagIndex];
		}

		return this;
	}

	virtual IMkState* disableFlag(eMkStateFlagType flagType) override
	{
		int flagIndex = (int)flagType;

		if (m_flags[flagIndex])
		{
			glDisable(g_glFlagTypeMapping[flagIndex]);
			m_flags[flagIndex] = false;

			MkStateLog(this) << "Disable Flag: " << g_glFlagName[flagIndex];
		}

		return this;
	}

	virtual bool isFlagEnabled(eMkStateFlagType flagType) const override
	{
		return m_flags[(int)flagType];
	}

	virtual IMkStateModifierPtr findParentModifier(IMkStateModifierPtr modifier) const override
	{
		if (modifier && m_parentState)
		{
			const GlState* parentGlState= static_cast<const GlState*>(m_parentState.get());

			// See if out parent state has a modifier with the same ID
			auto existingModifierIt = parentGlState->m_modifiers.find(modifier->getModifierID());
			if (existingModifierIt != parentGlState->m_modifiers.end())
			{
				return existingModifierIt->second;
			}

			// Recurse into the parent state to continue the search from there
			return m_parentState->findParentModifier(modifier);
		}

		return IMkStateModifierPtr();
	}

	virtual IMkState* addModifier(IMkStateModifierPtr modifier) override
	{
		if (modifier)
		{
			// Revert any existing modifier in this state
			auto existingModifierIt = m_modifiers.find(modifier->getModifierID());
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
			IMkStateModifierPtr parentModifier = findParentModifier(modifier);

			// Apply the new modifier
			modifier->apply(parentModifier);

			// Assign the modifier to this state
			m_modifiers[modifier->getModifierID()] = modifier;
		}

		return this;
	}

private:
	class MkStateStack& m_ownerStack;
	IMkStateConstPtr m_parentState;
	std::string m_scopeName;
	int m_stackDepth = -1;
	std::string m_debugPrefix;

	bool m_flags[(int)eMkStateFlagType::COUNT];
	std::map<std::string, IMkStateModifierPtr> m_modifiers;
};

IMkStatePtr createMkState(
	class MkStateStack& ownerStack,
	const std::string& scopeName,
	const int stackDepth)
{
	return std::make_shared<GlState>(ownerStack, scopeName, stackDepth);
}