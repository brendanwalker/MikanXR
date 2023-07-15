#pragma once

#include <vector>

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

enum class eGlStateFlagValue : unsigned char
{
	unset,
	enabled,
	disabled
};

class GlState
{
public:
	GlState(class GlStateStack& ownerStack, const int stackDepth);
	virtual ~GlState();

	inline GlStateStack& getOwnerStateStack() const { return m_ownerStack; }
	inline int getStackDepth() const { return m_stackDepth; }

	GlState& enableFlag(eGlStateFlagType flagType);
	GlState& disableFlag(eGlStateFlagType flagType);

private:
	class GlStateStack& m_ownerStack;
	const GlState* m_parentState;
	int m_stackDepth = -1;

	eGlStateFlagValue m_flags[(int)eGlStateFlagType::COUNT];
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
	GlStateStack() = default;
	virtual ~GlStateStack();

	GlState& pushState();
	void popState();

	int getCurrentStackDepth() const;
	GlState* getState(const int depth) const;

	GlScopedState createScopedState();

private:
	std::vector<class GlState*> m_stateStack;
};
