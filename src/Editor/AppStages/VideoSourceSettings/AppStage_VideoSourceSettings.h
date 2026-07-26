#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "MkRendererFwd.h"

#include <memory>
#include <vector>

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;

//-- definitions -----
class AppStage_VideoSourceSettings : public AppStage
{
public:
	AppStage_VideoSourceSettings(class IEditorWindow* ownerWindow);
	virtual ~AppStage_VideoSourceSettings();

	void setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent)
	{
		m_videoSourceComponent= videoSourceComponent;
	}

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void onGui() override;
	virtual void render(IMkViewportPtr targetViewport) override;

	static const char* APP_STAGE_NAME;

protected:
	// UI Events
	void onReturnEvent();

	// Remote Control
	bool handleRemoteControlCommand(const std::string& command, const std::vector<std::string>& parameters,
									std::vector<std::string>& outResults);
	bool handleGetVideoSourceComponentId(std::vector<std::string>& outResults);
	bool handleReturnRequest(std::vector<std::string>& outResults);

	VideoSourceComponentWeakPtr m_videoSourceComponent;
	VideoFrameDistortionViewPtr m_videoBufferView;

	// ARKit depth preview (debug/verification tool) - only selectable when the
	// current video source is ARKit, see ARKitVideoSourceComponent.
	bool m_bIsARKitVideoSource= false;
	enum class eDisplayMode
	{
		Color,              // normal color video
		DepthPreview,       // JBU-upsampled depth
		RawDepthPreview,    // raw low-res depth, nearest-upsampled (debug)
		MattePreview,       // raw incoming person matte, nearest-upsampled (debug)
		RefinedMattePreview // guided stencil-JBU person matte (crisp - the Human Stencil)
	};
	eDisplayMode m_displayMode= eDisplayMode::Color;

	// Pushes m_displayMode to the ARKit device's transient depth-preview selector (no-op
	// for non-ARKit sources). Called each frame from onGui() and reset on exit/pause.
	void applyDepthPreviewMode();
	IMkTriangulatedMeshPtr m_fullscreenDepthPreviewQuad;
};