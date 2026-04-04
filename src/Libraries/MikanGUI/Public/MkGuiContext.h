#pragma once

#include "MkGuiFwd.h"
#include "IMkWindowEventListener.h"

class MkGuiContext : public IMkWindowEventListener
{
public:
	MkGuiContext()= delete;
	MkGuiContext(SDL_Window* windowContext, void* glContext);
	virtual ~MkGuiContext();

	bool startup();
	void shutdown();
	void makeCurrent();

	struct ImFont* getNormalIconFont() const { return m_NormalIconFont; }
	struct ImFont* getBigIconFont() const { return m_BigIconFont; }

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const SDL_Event* event) override;

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