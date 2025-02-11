#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <string>

enum class eMkStateFlagType : int
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

class MIKAN_RENDERER_CLASS IMkState
{
public:
	virtual ~IMkState() {}

	virtual MkStateStack& getOwnerStateStack() const = 0;
	virtual int getStackDepth() const = 0;
	virtual const std::string& getDebugPrefix() const = 0;

	virtual IMkState* enableFlag(eMkStateFlagType flagType) = 0;
	virtual IMkState* disableFlag(eMkStateFlagType flagType) = 0;
	virtual bool isFlagEnabled(eMkStateFlagType flagType) const = 0;
	virtual IMkStateModifierPtr findParentModifier(IMkStateModifierPtr modifier) const = 0;
	virtual IMkState* addModifier(IMkStateModifierPtr modifier) = 0;
};
MIKAN_RENDERER_FUNC(IMkState*) createMkState(
	class MkStateStack& ownerStack,
	const std::string& scopeName,
	const int stackDepth);
MIKAN_RENDERER_FUNC(void) destroyMkState(IMkState* mkState);