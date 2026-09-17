#include "Shared/VideoSourceStatusGui.h"
#include "LocText.h"
#include "MikanVideoSourceTypes.h"
#include "VideoSourceComponent.h"

#include "imgui.h"

namespace VideoSourceStatusGui
{
void drawIntrinsicsWarning(VideoSourceComponentPtr videoSource)
{
	if (!videoSource)
		return;

	const eVideoSourceIntrinsicsStatus status= videoSource->getCameraIntrinsicsStatus();
	if (status == eVideoSourceIntrinsicsStatus::ok)
		return;

	const ImVec4 k_warnColor(1.f, 0.3f, 0.3f, 1.f);
	ImGui::PushStyleColor(ImGuiCol_Text, k_warnColor);

	if (status == eVideoSourceIntrinsicsStatus::uncalibrated)
	{
		ImGui::TextWrapped("%s", locText("componentPanel.intrinsicsUncalibrated"));
	}
	else
	{
		int liveWidth= 0, liveHeight= 0;
		videoSource->getVideoPixelDimensions(liveWidth, liveHeight);

		MikanVideoSourceIntrinsics intrinsics;
		videoSource->getCameraIntrinsics(intrinsics);
		const MikanMonoIntrinsics& monoIntrinsics= intrinsics.getMonoIntrinsics();

		ImGui::TextWrapped(locText("componentPanel.intrinsicsResolutionMismatchFmt"), (int)monoIntrinsics.pixel_width,
						   (int)monoIntrinsics.pixel_height, liveWidth, liveHeight);
	}

	ImGui::PopStyleColor();
}
} // namespace VideoSourceStatusGui
