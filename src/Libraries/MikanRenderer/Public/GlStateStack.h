#pragma once

#include "IMkStateModifier.h"
#include "Logger.h"

#include <memory>
#include <string>
#include <vector>
#include <map>

enum class eGlStateFlagType : int
{
	INVALID= -1,

	light0,
	texture2d,
	depthTest,
	stencilTest,
	scissorTest,
	blend,
	cullFace,
	programPointSize,

	COUNT
};

class GlState
{
public:
	GlState(class GlStateStack& ownerStack, const std::string& scopeName, const int stackDepth);
	virtual ~GlState();

	inline GlStateStack& getOwnerStateStack() const { return m_ownerStack; }
	inline int getStackDepth() const { return m_stackDepth; }
	inline const std::string& getDebugPrefix() const { return m_debugPrefix; }

	GlState& enableFlag(eGlStateFlagType flagType);
	GlState& disableFlag(eGlStateFlagType flagType);
	bool isFlagEnabled(eGlStateFlagType flagType) const;
	MkStateModifierPtr findParentModifier(MkStateModifierPtr modifier) const;
	GlState& addModifier(MkStateModifierPtr modifier);

private:
	class GlStateStack& m_ownerStack;
	const GlState* m_parentState;
	std::string m_scopeName;
	int m_stackDepth = -1;
	std::string m_debugPrefix;

	bool m_flags[(int)eGlStateFlagType::COUNT];
	std::map<std::string, MkStateModifierPtr> m_modifiers;
};

class GlScopedState
{
public:
	GlScopedState(class GlState& state);
	virtual ~GlScopedState();

	inline GlState& getStackState() const { return m_state; }
	inline int getStackDepth() const { return m_state.getStackDepth(); }

private:
	GlState& m_state;
};

class GlStateStack
{
public:
	GlStateStack() = delete;
	GlStateStack(class IMkWindow* ownerWindow);
	virtual ~GlStateStack();

	GlState& pushState(const std::string& scopeName);
	void popState();

	int getCurrentStackDepth() const;
	GlState* getState(const int depth) const;

	inline GlState* getCurrentState() const { return getState(getCurrentStackDepth()); }
	inline class IMkWindow* getOwnerWindow() const { return m_ownerWindow; }

	inline void setDebugPrintEnabled(bool bDebugPrint) { m_bDebugPrint = bDebugPrint; }
	inline bool isDebugPrintEnabled() const { return m_bDebugPrint; }

	GlScopedState createScopedState(const std::string& scopeName);

private:
	class IMkWindow* m_ownerWindow= nullptr;
	std::vector<class GlState*> m_stateStack;
	bool m_bDebugPrint= false;
};


class GlStateLog
{
public:
	GlStateLog(const GlState& state)
		: m_loggerStream(LogSeverityLevel::info)
		, m_state(state) 
	{}

	template<class T>
	GlStateLog& operator<<(const T& x)
	{
		if (m_state.getOwnerStateStack().isDebugPrintEnabled())
		{
			m_loggerStream << m_state.getDebugPrefix() << x;
		}

		return *this;
	}

protected:
	LoggerStream m_loggerStream;
	const GlState& m_state;
};