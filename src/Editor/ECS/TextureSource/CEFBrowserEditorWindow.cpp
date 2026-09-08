//-- includes -----
#include "CEFBrowserEditorWindow.h"

#include "App.h"
#include "CEFTextureSourceComponent.h"
#include "CEFTextureSourceSystem.h"
#include "IMkGraphicsContext.h"
#include "IMkLineRenderer.h"
#include "IMkState.h"
#include "IMkTexture.h"
#include "IMkTriangulatedMesh.h"
#include "LocText.h"
#include "MainWindow.h"
#include "MikanTextRenderer.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkGuiContext.h"
#include "MkGuiScopedUpdate.h"
#include "MkGuiScopedWindow.h"
#include "MkScopedState.h"
#include "MkStateModifiers.h"
#include "MkStateStack.h"
#include "MkWindowEvent.h"
#include "MulticastDelegate.h"
#include "ProjectManager.h"

#include "imgui.h"

#include <easy/profiler.h>

#include <algorithm>

//-- constants -----
static const int k_browser_window_default_width= 1280;
static const int k_browser_window_default_height= 720;

// Toolbar height used for the first frame's letterbox, before drawToolbar has measured the real one
// from the active ImGui style. Only a seed: a hard-coded height clips the toolbar's contents as soon
// as the font or frame padding changes.
static const float k_toolbar_height_seed= 32.f;

//-- public methods -----
CEFBrowserEditorWindow::CEFBrowserEditorWindow(App* ownerApp)
	: EditorWindow(ownerApp)
{
	// The browser's color texture is created on the main window's GL context in
	// CEFTextureSourceComponent::update, so this window has to share that context to sample it.
	shareGraphicsContextWithMainWindow();
}

bool CEFBrowserEditorWindow::startup()
{
	EASY_FUNCTION();

	bool success= true;

	if (success
		&& !startupWindow(locText("cefBrowser.windowTitle"), k_browser_window_default_width,
						  k_browser_window_default_height))
	{
		success= false;
	}

	if (success && !startupGuiContext("cef_browser"))
	{
		success= false;
	}

	if (success && !startupStyleManager())
	{
		success= false;
	}

	if (success && !startupTextureCache())
	{
		success= false;
	}

	// The browser paints BGRA, so this uses the same vertically flipped RGBA quad that the texture
	// source settings stage uses to display this texture.
	if (success)
	{
		m_pageQuad= createFullscreenQuadMesh(m_graphicsContext.get(), true, true);
	}

	// Close this window if the component it is bound to goes away
	auto cefSystem= getProjectManager()->getSystemOfType<CEFTextureSourceSystem>();
	if (cefSystem)
	{
		cefSystem->OnComponentDisposed+= MakeDelegate(this, &CEFBrowserEditorWindow::onTextureSourceComponentDisposed);
	}

	return success;
}

bool CEFBrowserEditorWindow::bindTextureSourceComponent(CEFTextureSourceComponentPtr textureSourceComponent)
{
	m_textureSourceComponent= textureSourceComponent;
	m_boundComponentId= textureSourceComponent ? textureSourceComponent->getComponentId() : -1;

	if (textureSourceComponent)
	{
		const std::string& url= textureSourceComponent->getCEFTextureSourceDefinition()->getUrl();
		strncpy_s(m_urlBuffer, sizeof(m_urlBuffer), url.c_str(), _TRUNCATE);

		setTitle(locFormat("cefBrowser.windowTitleFmt", textureSourceComponent->getName().c_str()));
	}

	return true;
}

void CEFBrowserEditorWindow::onTextureSourceComponentDisposed(MikanObjectSystemPtr objectSystem,
															  MikanComponentConstPtr component)
{
	if (component && component->getComponentId() == m_boundComponentId)
	{
		m_mkWindowContext->requestClose();
	}
}

float CEFBrowserEditorWindow::getToolbarHeight() const
{
	return m_toolbarHeight > 0.f ? m_toolbarHeight : k_toolbar_height_seed;
}

void CEFBrowserEditorWindow::getPageAreaSize(int& outWidth, int& outHeight) const
{
	outWidth= (int)m_mkWindowContext->getWidth();
	outHeight= std::max((int)(m_mkWindowContext->getHeight() - getToolbarHeight()), 0);
}

CEFBrowserEditorWindow::PageRect CEFBrowserEditorWindow::computePageRect() const
{
	PageRect pageRect;

	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (!component)
		return pageRect;

	int areaWidth= 0;
	int areaHeight= 0;
	getPageAreaSize(areaWidth, areaHeight);
	if (areaWidth <= 0 || areaHeight <= 0)
		return pageRect;

	const auto def= component->getCEFTextureSourceDefinition();
	const int browserWidth= def->getWidth();
	const int browserHeight= def->getHeight();
	if (browserWidth <= 0 || browserHeight <= 0)
		return pageRect;

	// Letterbox rather than resizing the browser: the definition's resolution is what the scene
	// texture is authored at, so dragging this window must not change the compositor's output.
	const float browserAspect= (float)browserWidth / (float)browserHeight;
	const float areaAspect= (float)areaWidth / (float)areaHeight;

	if (areaAspect > browserAspect)
	{
		pageRect.height= areaHeight;
		pageRect.width= (int)(areaHeight * browserAspect);
	}
	else
	{
		pageRect.width= areaWidth;
		pageRect.height= (int)(areaWidth / browserAspect);
	}

	pageRect.x= (areaWidth - pageRect.width) / 2;
	pageRect.y= (int)getToolbarHeight() + (areaHeight - pageRect.height) / 2;

	return pageRect;
}

bool CEFBrowserEditorWindow::windowToBrowserPosition(int windowX, int windowY, int& outBrowserX, int& outBrowserY) const
{
	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (!component)
		return false;

	const PageRect pageRect= computePageRect();
	if (pageRect.width <= 0 || pageRect.height <= 0)
		return false;

	const bool bInsidePage= windowX >= pageRect.x && windowX < pageRect.x + pageRect.width && windowY >= pageRect.y
							&& windowY < pageRect.y + pageRect.height;

	// Coordinates come back clamped even when the cursor is outside, so a drag that runs off the
	// page edge keeps delivering sensible positions instead of stalling at the boundary.
	const auto def= component->getCEFTextureSourceDefinition();
	const int clampedX= std::clamp(windowX, pageRect.x, pageRect.x + pageRect.width - 1);
	const int clampedY= std::clamp(windowY, pageRect.y, pageRect.y + pageRect.height - 1);

	outBrowserX= ((clampedX - pageRect.x) * def->getWidth()) / pageRect.width;
	outBrowserY= ((clampedY - pageRect.y) * def->getHeight()) / pageRect.height;

	return bInsidePage;
}

void CEFBrowserEditorWindow::setBrowserFocus(bool bFocused)
{
	if (m_bBrowserHasFocus == bFocused)
		return;

	m_bBrowserHasFocus= bFocused;

	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (component)
	{
		component->setBrowserFocus(bFocused);
	}
}

void CEFBrowserEditorWindow::update(float deltaSeconds)
{
	EASY_FUNCTION();

	// Push ImGui update scope (handles events, builds draw lists)
	MkGuiScopedUpdate scopedCtx(*m_guiContext);

	// Process SDL events
	m_mkWindowContext->handleEvents(this);

	// Follow the page's own title once it has one
	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (component)
	{
		const std::string& browserTitle= component->getBrowserTitle();
		if (!browserTitle.empty() && browserTitle != getTitle())
		{
			setTitle(browserTitle);
		}

		// Track navigation in the URL field unless the user is part way through typing a new one
		if (!m_bUrlFieldActive)
		{
			const std::string& browserUrl= component->getBrowserUrl();
			if (!browserUrl.empty() && browserUrl != m_urlBuffer)
			{
				strncpy_s(m_urlBuffer, sizeof(m_urlBuffer), browserUrl.c_str(), _TRUNCATE);
			}
		}
	}

	// The toolbar is built here rather than in render() because MkGuiScopedUpdate finalizes the
	// ImGui draw lists when it leaves scope at the end of this function. render() only submits them.
	drawToolbar();
}

void CEFBrowserEditorWindow::drawToolbar()
{
	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (!component)
		return;

	// Fit the strip to one row of frame-height widgets under the current style, rather than a fixed
	// height that clips the buttons and the URL field once the font or frame padding grows.
	m_toolbarHeight= ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2.f;

	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(m_mkWindowContext->getWidth(), m_toolbarHeight), ImGuiCond_Always);

	constexpr ImGuiWindowFlags k_flags= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
										| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
										| ImGuiWindowFlags_NoScrollbar;

	MkGuiScopedWindow toolbar("##CEFBrowserToolbar", nullptr, k_flags);
	if (!toolbar)
		return;

	ImGui::BeginDisabled(!component->getBrowserCanGoBack());
	if (ImGui::Button(locLabel("cefBrowser.back")))
		component->browserGoBack();
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(!component->getBrowserCanGoForward());
	if (ImGui::Button(locLabel("cefBrowser.forward")))
		component->browserGoForward();
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button(locLabel("cefBrowser.reload")))
		component->browserReload();

	ImGui::SameLine();
	if (ImGui::Button(locLabel("cefBrowser.devTools")))
		component->browserShowDevTools();

	// The URL field takes whatever width is left over
	ImGui::SameLine();
	ImGui::SetNextItemWidth(std::max(ImGui::GetContentRegionAvail().x, 1.f));
	if (ImGui::InputText("##CEFBrowserUrl", m_urlBuffer, sizeof(m_urlBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		// Typing a URL drives the live browser. It reaches the definition only when the component's
		// URL write-back property is enabled, via OnAddressChange.
		component->browserLoadUrl(m_urlBuffer);
		setBrowserFocus(true);
	}
	m_bUrlFieldActive= ImGui::IsItemActive();
}

void CEFBrowserEditorWindow::render()
{
	EASY_FUNCTION();

	IMkGraphicsContext* gfx= m_graphicsContext.get();
	MkStateStack& stateStack= gfx->getMkStateStack();

	// Clear the window
	gfx->renderBegin();

	// --- Layer 1: the browser page, letterboxed ---
	{
		MkScopedState scopedState= stateStack.createScopedState("CEFBrowser renderPage");
		IMkState* glState= scopedState.getStackState();

		CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
		IMkTexturePtr pageTexture=
			component ? component->getClientColorSourceTexture(INVALID_MIKAN_ID, eTextureSourceColorType::colorRGBA)
					  : IMkTexturePtr();

		const PageRect pageRect= computePageRect();

		if (pageTexture && m_pageQuad && pageRect.width > 0 && pageRect.height > 0)
		{
			// The page rect is measured top-down from the window's upper left, but the GL viewport
			// is bottom-up, so flip the origin on the way in.
			const int viewportY= (int)m_mkWindowContext->getHeight() - (pageRect.y + pageRect.height);
			mkStateSetViewport(glState, pageRect.x, viewportY, pageRect.width, pageRect.height);

			MkMaterialInstancePtr materialInstance= m_pageQuad->getMaterialInstance();
			MkMaterialConstPtr material= materialInstance->getMaterial();

			if (auto materialBinding= material->bindMaterial())
			{
				materialInstance->setTextureBySemantic(eUniformSemantic::rgbaTexture, pageTexture);

				if (auto materialInstanceBinding= materialInstance->bindMaterialInstance(materialBinding))
				{
					glState->disableFlag(eMkStateFlagType::depthTest);
					m_pageQuad->drawElements();
				}
			}
		}
	}

	// --- Layer 2: toolbar ---
	{
		MkScopedState scopedState= stateStack.createScopedState("CEFBrowser renderUI");
		IMkState* glState= scopedState.getStackState();
		mkStateSetViewport(glState, 0, 0, (int)m_mkWindowContext->getWidth(), (int)m_mkWindowContext->getHeight());

		m_guiContext->submitDrawData();
	}

	gfx->getLineRenderer()->render(true);
	gfx->getTextRenderer()->render();

	// Finalize rendering
	gfx->renderEnd();

	// Present the rendered frame
	presentFrame();
}

void CEFBrowserEditorWindow::shutdown()
{
	auto cefSystem= getProjectManager()->getSystemOfType<CEFTextureSourceSystem>();
	if (cefSystem)
	{
		cefSystem->OnComponentDisposed-= MakeDelegate(this, &CEFBrowserEditorWindow::onTextureSourceComponentDisposed);
	}

	// Hand keyboard focus back before the window goes away
	setBrowserFocus(false);

	m_textureSourceComponent.reset();
	m_boundComponentId= -1;
	m_pageQuad= nullptr;

	shutdownTextureCache();
	shutdownStyleManager();
	shutdownGuiContext();
	shutdownWindow();
}

// -- IMkWindowEventListener
bool CEFBrowserEditorWindow::onWindowEvent(const MkWindowEvent& event)
{
	// ImGui sees every event, so its own input state stays coherent whoever ends up acting on it.
	// Its capture flags still decide the keyboard, but not the mouse: the page area is not an ImGui
	// window, so geometry decides where a click goes. WantCaptureMouse is also a frame stale by
	// construction, and deferring to it here would eat the first page click after every toolbar
	// press, which is both a real interaction bug and one that hides the page from a driven session.
	const bool bGuiWantsEvent= m_guiContext->onWindowEvent(event);

	CEFTextureSourceComponentPtr component= m_textureSourceComponent.lock();
	if (!component || !component->hasBrowser())
		return bGuiWantsEvent;

	int browserX= 0;
	int browserY= 0;

	switch (event.getEventType())
	{
	case eMkWindowEventType::MouseMotion:
	{
		const bool bInsidePage= windowToBrowserPosition(event.getMouseX(), event.getMouseY(), browserX, browserY);

		// A drag in progress keeps tracking past the page edge, so selecting text or dragging a
		// slider does not stall the moment the cursor crosses into the letterbox bars.
		const bool bDragging=
			m_inputState.bLeftButtonDown || m_inputState.bMiddleButtonDown || m_inputState.bRightButtonDown;

		component->sendMouseMove(browserX, browserY, !bInsidePage && !bDragging, m_inputState);
		return bInsidePage;
	}

	case eMkWindowEventType::MouseButtonDown:
	case eMkWindowEventType::MouseButtonUp:
	{
		const bool bMouseUp= event.getEventType() == eMkWindowEventType::MouseButtonUp;
		const int mouseButton= event.getMouseButton();

		bool bWasButtonDown= false;
		switch (mouseButton)
		{
		case MkMouseButton::LEFT:
			bWasButtonDown= m_inputState.bLeftButtonDown;
			m_inputState.bLeftButtonDown= !bMouseUp;
			break;
		case MkMouseButton::MIDDLE:
			bWasButtonDown= m_inputState.bMiddleButtonDown;
			m_inputState.bMiddleButtonDown= !bMouseUp;
			break;
		case MkMouseButton::RIGHT:
			bWasButtonDown= m_inputState.bRightButtonDown;
			m_inputState.bRightButtonDown= !bMouseUp;
			break;
		}

		const bool bInsidePage= windowToBrowserPosition(event.getMouseX(), event.getMouseY(), browserX, browserY);

		// A release always reaches a browser that saw the matching press, even if the cursor has
		// since left the page, or the page is left stuck mid-drag.
		if (!bInsidePage && !(bMouseUp && bWasButtonDown))
		{
			// Pressing on the letterbox bars or the toolbar strip takes focus off the page
			if (!bMouseUp)
			{
				setBrowserFocus(false);
			}
			return bGuiWantsEvent;
		}

		if (!bMouseUp)
		{
			setBrowserFocus(true);
		}

		component->sendMouseButton(browserX, browserY, mouseButton, bMouseUp, event.getMouseClickCount(), m_inputState);
		return bInsidePage;
	}

	case eMkWindowEventType::MouseWheel:
	{
		if (!windowToBrowserPosition(event.getMouseX(), event.getMouseY(), browserX, browserY))
			return bGuiWantsEvent;

		component->sendMouseWheel(browserX, browserY, event.getMouseWheelScrollAmountX(),
								  event.getMouseWheelScrollAmount(), m_inputState);
		return true;
	}

	case eMkWindowEventType::KeyDown:
	case eMkWindowEventType::KeyUp:
	{
		m_inputState.keyMod= event.getKeyMod();

		// The keyboard does defer to ImGui: the URL field is a real ImGui widget, and when it holds
		// focus the keystrokes are its own.
		if (bGuiWantsEvent || !m_bBrowserHasFocus)
			return bGuiWantsEvent;

		component->sendKeyEvent(event.getKeySym(), event.getEventType() == eMkWindowEventType::KeyDown, m_inputState);
		return true;
	}

	case eMkWindowEventType::TextInput:
	{
		if (bGuiWantsEvent || !m_bBrowserHasFocus)
			return bGuiWantsEvent;

		component->sendTextInput(event.getText(), m_inputState);
		return true;
	}

	case eMkWindowEventType::WindowEvent:
	{
		if (event.getWindowEventID() == eMkWindowEventID::FocusLost)
		{
			// Modifier and button state cannot be observed while another window has focus, so drop
			// it rather than carrying a stale chord back in.
			m_inputState= CEFBrowserInputState();
			setBrowserFocus(false);
		}
		return bGuiWantsEvent;
	}

	default:
		return bGuiWantsEvent;
	}
}
