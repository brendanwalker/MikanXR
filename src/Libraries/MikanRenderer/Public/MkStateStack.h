#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "MkScopedState.h"

#include <string>

class MIKAN_RENDERER_CLASS MkStateStack
{
public:
	MkStateStack() = delete;
	MkStateStack(IMkWindow* ownerWindow);
	virtual ~MkStateStack();

	IMkState* pushState(const std::string& scopeName);
	void popState();

	int getCurrentStackDepth() const;
	IMkState* getState(const int depth) const;

	IMkState* getCurrentState() const;
	IMkWindow* getOwnerWindow() const;

	void setDebugPrintEnabled(bool bDebugPrint);
	bool isDebugPrintEnabled() const;

	MkScopedState createScopedState(const std::string& scopeName);

private:
	struct MkStateStackData* m_data;
};
