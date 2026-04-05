#pragma once

#include "MkGuiFwd.h"

class MkGuiScopedContext
{
public:
	MkGuiScopedContext() = delete;
	MkGuiScopedContext(MkGuiContext& context);
	~MkGuiScopedContext();

private:
	struct ImGuiContext* m_prevImGuiContext= nullptr;
	struct ImNodesContext* m_prevImNodesContext= nullptr;
};
