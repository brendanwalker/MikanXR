#pragma once

#include "MkGuiScopedContext.h"

class MkGuiScopedRender : public MkGuiScopedContext
{
public:
	MkGuiScopedRender() = delete;
	MkGuiScopedRender(MkGuiContext& context);

	virtual ~MkGuiScopedRender();
};
