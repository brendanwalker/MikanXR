#pragma once

//-- includes -----
#include "CEFBrowserInput.h"
#include "ComponentFwd.h"
#include "EditorWindow.h"
#include "MikanRendererFwd.h"
#include "ObjectSystemFwd.h"

#include <memory>

//-- definitions -----
// Satellite window that makes a windowless CEF texture source interactive. It draws the browser's
// scene texture letterboxed under a toolbar and forwards mouse and keyboard input back into the
// browser, so the page stays live in the scene while the main window keeps running the Project
// app stage. One window per component, bound by component id.
class CEFBrowserEditorWindow : public EditorWindow
{
public:
	CEFBrowserEditorWindow(class App* ownerApp);

	bool bindTextureSourceComponent(CEFTextureSourceComponentPtr textureSourceComponent);
	inline int getBoundComponentId() const { return m_boundComponentId; }

	// Size of the region the page is fitted into, with the toolbar strip excluded. This is what the
	// component's "match resolution" function copies into the definition, so that afterwards the
	// page fills the region exactly and the letterbox bars close.
	void getPageAreaSize(int& outWidth, int& outHeight) const;

	// -- IEditorWindow --
	virtual bool startup() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual bool getIsRenderingStage() const override { return false; }
	virtual IMkViewportPtr getRenderingViewport() const override { return nullptr; }

	// -- IMkWindowEventListener
	virtual bool onWindowEvent(const class MkWindowEvent& event) override;

private:
	// Page rectangle in window client pixels, top-down from the upper left corner
	struct PageRect
	{
		int x= 0;
		int y= 0;
		int width= 0;
		int height= 0;
	};

	float getToolbarHeight() const;
	PageRect computePageRect() const;
	bool windowToBrowserPosition(int windowX, int windowY, int& outBrowserX, int& outBrowserY) const;
	void setBrowserFocus(bool bFocused);
	void drawToolbar();
	void onTextureSourceComponentDisposed(MikanObjectSystemPtr objectSystem, MikanComponentConstPtr component);

	CEFTextureSourceComponentWeakPtr m_textureSourceComponent;
	int m_boundComponentId= -1;

	IMkTriangulatedMeshPtr m_pageQuad;

	// Modifier and button state carried with each forwarded input event. MkWindowEvent reports
	// modifiers only on key events and never reports button state on motion, so both are tracked
	// here across events and reset whenever this window loses focus.
	CEFBrowserInputState m_inputState;
	bool m_bBrowserHasFocus= false;

	// Measured by drawToolbar from the live ImGui style each frame, since the strip has to fit a
	// button row whose height follows the font and frame padding. The letterbox math reads it.
	float m_toolbarHeight= 0.f;

	char m_urlBuffer[2048]= {};
	// Set while the user is editing the URL field, so live address changes from the browser do not
	// overwrite what is being typed.
	bool m_bUrlFieldActive= false;
};
