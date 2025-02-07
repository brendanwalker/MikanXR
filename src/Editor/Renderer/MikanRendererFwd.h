#pragma once

#include "MkRendererFwd.h"
#include <memory>

class GlRmlUiRender;
using GlRmlUiRenderUniquePtr = std::unique_ptr<GlRmlUiRender>;

class MikanCamera;
using GlCameraPtr = std::shared_ptr<MikanCamera>;
using GlCameraConstPtr = std::shared_ptr<const MikanCamera>;

class GlScene;
using GlScenePtr = std::shared_ptr<GlScene>;
using GlSceneWeakPtr = std::weak_ptr<GlScene>;
using GlSceneConstPtr = std::shared_ptr<const GlScene>;

class MikanViewport;
using GlViewportPtr = std::shared_ptr<MikanViewport>;
using GlViewportWeakPtr = std::weak_ptr<MikanViewport>;
using GlViewportConstPtr = std::shared_ptr<const MikanViewport>;

class MikanShaderCache;
using MikanShaderCacheUniquePtr = std::unique_ptr<MikanShaderCache>;