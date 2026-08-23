#pragma once

//-- includes -----
#include "EditorWindow.h"

//-- definitions -----
// Lists every HTTP route currently registered on the HttpInterprocessMessageServer
// (see ScriptRequestHandler::registerHttpTriggerRoute) as a clickable button, so triggers
// can be fired from inside the editor without an external HTTP client.
class HttpTriggerWindow : public EditorWindow
{
public:
	HttpTriggerWindow(class App* ownerApp);

	// -- IEditorWindow ----
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual bool getIsRenderingStage() const override { return false; }
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const class MkWindowEvent& event) override;

protected:
	void updateUI();
};
