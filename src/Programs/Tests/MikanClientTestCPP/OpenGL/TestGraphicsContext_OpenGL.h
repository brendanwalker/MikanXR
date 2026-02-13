#pragma once

#include "MkRendererFwd.h"
#include "ITestGraphicsContext.h"

class TestGraphicsContext_OpenGL : public ITestGraphicsContext
{
public:
	TestGraphicsContext_OpenGL();
	
	MkStateStack& getMkStateStack();

	virtual MikanClientGraphicsApi getGraphicsApi() const override { return MikanClientGraphicsApi_OpenGL; } 
	virtual TestCameraRenderTargetPtr allocateCameraRenderTarget(IMikanAPIPtr mikanAPI, int cameraId) override;
	virtual bool create(int windowWidth, int windowHeight) override;
	virtual void render() const override;
	virtual void dispose() override;

private:
	bool m_sdlInitialized= false;

	int m_windowWidth = 0;
	int m_windowHeight = 0;
	struct SDL_Window* m_sdlWindow = nullptr;

	void* m_glContext= 0;
	MkStateStackUniquePtr m_mkStateStack;
};