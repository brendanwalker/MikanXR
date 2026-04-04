#pragma once

#include "MkGuiScopedContext.h"

class MkGuiScopedUpdate : public MkGuiScopedContext
{
public:
	MkGuiScopedUpdate() = delete;
	MkGuiScopedUpdate(MkGuiContext& context);
	~MkGuiScopedUpdate();
};
