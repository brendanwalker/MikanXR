#pragma once

#include "MkGuiFwd.h"

class MkGuiContext
{
public:
	MkGuiContext()= delete;
	MkGuiContext(SDL_Window* windowContext, void* glContext);
	virtual ~MkGuiContext();

	bool startup();
	void shutdown();
	void makeCurrent();

protected:
	void configImGui();
	void configImNodes();
	
private:
	SDL_Window* m_windowContext= nullptr;
	void* m_glContext= nullptr;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont = nullptr;
	struct ImFont* m_BigIconFont = nullptr;
	bool m_imguiSDLBackendInitialised= false;
	bool m_imguiOpenGLBackendInitialised= false;
};