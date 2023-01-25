#pragma once

#include <vector>

enum class eGlStateFlagType : int
{
	INVALID= -1,

	light0,
	texture2d,
	depthTest,
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

class GLState
{
public:
	GLState(class GlStateStack& ownerStack, const int stackDepth);
	virtual ~GLState();

	inline GlStateStack& getOwnerStateStack() const { return m_ownerStack; }
	inline int getStackDepth() const { return m_stackDepth; }

	GLState& enableFlag(eGlStateFlagType flagType);
	GLState& disableFlag(eGlStateFlagType flagType);

private:
	class GlStateStack& m_ownerStack;
	const GLState* m_parentState;
	int m_stackDepth = -1;

	eGlStateFlagValue m_flags[(int)eGlStateFlagType::COUNT];
};

class GLScopedState
{
public:
	GLScopedState(class GLState& state);
	virtual ~GLScopedState();

	inline GLState& getStackState() const { return m_state; }
	inline int getStackDepth() const { return m_state.getStackDepth(); }

private:
	GLState& m_state;
};

class GlStateStack
{
public:
	GlStateStack() = default;
	virtual ~GlStateStack();

	GLState& createState();
	GLState* getState(const int depth) const;
	void freeState(GLState& state);

	GLScopedState createScopedState();

private:
	std::vector<class GLState*> m_stateStack;
};
