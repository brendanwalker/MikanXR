#pragma once

#include "ComponentFwd.h"

namespace VideoSourceStatusGui
{
// A warning line when the source's stored intrinsics do not describe the video
// mode it is running: never calibrated, or calibrated at another resolution.
// Draws nothing when they match.
void drawIntrinsicsWarning(VideoSourceComponentPtr videoSource);
} // namespace VideoSourceStatusGui
