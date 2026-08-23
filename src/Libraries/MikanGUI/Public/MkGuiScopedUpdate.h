#pragma once

#include "MkGuiExport.h"
#include "MkGuiScopedContext.h"

class MIKAN_GUI_CLASS MkGuiScopedUpdate : public MkGuiScopedContext
{
public:
	MkGuiScopedUpdate()= delete;
	MkGuiScopedUpdate(MkGuiContext& context);
	~MkGuiScopedUpdate();
};
