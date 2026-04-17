#pragma once

#include "MkGuiFwd.h"
#include "IMkWindow.h"
#include "IMkWindowEventListener.h"
#include "IMkGraphicsContext.h"

class MkGuiContext : public IMkWindowEventListener
{
public:
	MkGuiContext()= delete;
	MkGuiContext(class IMkWindow* window);
	virtual ~MkGuiContext();

	bool startup();
	void shutdown();
	void makeCurrent();
	void submitDrawData();

	struct ImFont* getNormalIconFont() const { return m_NormalIconFont; }
	struct ImFont* getBigIconFont() const { return m_BigIconFont; }

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const MkWindowEvent& event) override;

protected:
	bool initImGuiSDLBackend();
	bool initImGuiOpenGlBackend();

	void configImGui();
	void configImNodes();

private:
	class IMkWindow* m_window= nullptr;
	struct ImGuiContext* m_imguiContext= nullptr;
	struct ImNodesContext* m_imnodesContext= nullptr;
	struct ImFont* m_NormalIconFont = nullptr;
	struct ImFont* m_BigIconFont = nullptr;
	eWindowAPI m_imguiWindowAPI = eWindowAPI::INVALID;
	eGraphicsAPI m_imguiGraphicsAPI = eGraphicsAPI::INVALID;
};