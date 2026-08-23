#pragma once

#include "MkWindowExport.h"
#include "MkWindowFwd.h"
#include "MkRendererFwd.h"

#include <string>

enum class eWindowAPI
{
	INVALID= -1,

	SDL= 0,

	COUNT
};

class IMkWindowContext
{
public:
	virtual ~IMkWindowContext() {}

	virtual bool startup()= 0;
	virtual void update(float deltaSeconds)= 0;
	virtual void render()= 0;
	virtual void present()= 0;
	virtual void shutdown()= 0;

	virtual void enableGLDataSharing() {}  // enable GL resource sharing before startup()
	virtual void useExistingGLContext() {} // attach to the currently active GL context instead of creating a new one

	virtual const char* getTitle() const= 0;
	virtual float getWidth() const= 0;
	virtual float getHeight() const= 0;
	virtual float getAspectRatio() const= 0;
	virtual bool getIsRenderingStage() const= 0;

	virtual void getMouseScreenPosition(int& outScreenX, int& outScreenY) const= 0;

	virtual eWindowAPI getWindowAPI() const= 0;
	virtual void* getNativeWindowHandle() const= 0;
	virtual IMkGraphicsContextPtr getGraphicsContext() const= 0;
	virtual IMkViewportPtr getRenderingViewport() const= 0;

	virtual void makeContextCurrent()= 0;
	virtual bool wantsDestroy() const= 0;
	virtual void requestClose() {}

	virtual void setTitle(const std::string& title)= 0;
	virtual void setSize(int width, int height)= 0;

	virtual void handleEvents(class IMkWindowEventListener* eventListener)= 0;
	virtual bool hasMouseFocus() const= 0;
	virtual bool hasKeyboardFocus() const= 0;
};

MIKAN_WINDOW_FUNC(IMkWindowContextPtr) createMkWindowContext(IMkWindowContextManagerPtr ownerWindowManager,
															 IMkGraphicsContextPtr graphicsContext);
