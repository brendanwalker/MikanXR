#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"

class MIKAN_RENDERER_CLASS MkScopedState
{
public:
	MkScopedState(IMkStatePtr state);
	virtual ~MkScopedState();

	IMkStatePtr getStackState() const;
	int getStackDepth() const;

private:
	// Private implementation needed hide std::shared_ptr from DLL export
	struct MkScopedStateImpl* m_impl;
};
