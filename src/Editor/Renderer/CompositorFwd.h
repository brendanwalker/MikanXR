#pragma once

#include <memory>

class CompositorPreset;
using CompositorPresetPtr = std::shared_ptr<CompositorPreset>;
using CompositorPresetConstPtr = std::shared_ptr<const CompositorPreset>;

class GlFrameCompositorConfig;
using GlFrameCompositorConfigPtr = std::shared_ptr<GlFrameCompositorConfig>;
using GlFrameCompositorConfigConstPtr = std::shared_ptr<const GlFrameCompositorConfig>;