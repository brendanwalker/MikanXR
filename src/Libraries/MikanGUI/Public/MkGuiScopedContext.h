#pragma once

#include "MkGuiExport.h"
#include "MkGuiFwd.h"

class MIKAN_GUI_CLASS MkGuiScopedContext
{
public:
	MkGuiScopedContext()= delete;
	MkGuiScopedContext(MkGuiContext& context);
	~MkGuiScopedContext();

private:
	struct ImGuiContext* m_prevImGuiContext= nullptr;
	struct ImNodesContext* m_prevImNodesContext= nullptr;
};
