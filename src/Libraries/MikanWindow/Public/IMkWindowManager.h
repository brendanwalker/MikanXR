#pragma once

#include "MkWindowExport.h"

#include <memory>
#include <string>

class IMkWindow;

class IMkWindowManager;
using IMkWindowManagerPtr = std::shared_ptr<IMkWindowManager>;

class MIKAN_WINDOW_CLASS IMkWindowManager
{
public:
	virtual ~IMkWindowManager() {}

	virtual bool startup() = 0;
	virtual void shutdown() = 0;
	virtual void pollEvents() = 0;

	virtual bool getIsInitialized() const = 0;
	virtual const std::string& getGlslVersion() const = 0;
	virtual void setMouseCursor(const std::string& cursorName) = 0;

	virtual void pushCurrentWindowContext(IMkWindow* window) = 0;
	virtual IMkWindow* getCurrentWindowContext() const = 0;
	virtual void popCurrentWindowContext(IMkWindow* window) = 0;
};

MIKAN_WINDOW_FUNC(IMkWindowManagerPtr) createMkWindowManager();
