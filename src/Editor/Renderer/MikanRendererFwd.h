#pragma once

#include "MkRendererFwd.h"
#include <memory>

class GlRmlUiRender;
using GlRmlUiRenderUniquePtr = std::unique_ptr<GlRmlUiRender>;

class MikanCamera;
using MikanCameraPtr = std::shared_ptr<MikanCamera>;
using MikanCameraWeakPtr = std::weak_ptr<MikanCamera>;
using MikanCameraConstPtr = std::shared_ptr<const MikanCamera>;

class MkScene;
using MkScenePtr = std::shared_ptr<MkScene>;
using MkSceneWeakPtr = std::weak_ptr<MkScene>;
using MkSceneConstPtr = std::shared_ptr<const MkScene>;

class MikanViewport;
using MikanViewportPtr = std::shared_ptr<MikanViewport>;
using MikanViewportUniquePtr = std::unique_ptr<MikanViewport>;
using MikanViewportWeakPtr = std::weak_ptr<MikanViewport>;
using MikanViewportConstPtr = std::shared_ptr<const MikanViewport>;

class MikanShaderCache;
using MikanShaderCacheUniquePtr = std::unique_ptr<MikanShaderCache>;

class MikanTextureCache;
using MikanTextureCacheUniquePtr = std::unique_ptr<MikanTextureCache>;

class MikanShaderConfig;

class MikanModelResourceManager;
using MikanModelResourceManagerSharedPtr = std::shared_ptr<MikanModelResourceManager>;
using MikanModelResourceManagerUniquePtr = std::unique_ptr<MikanModelResourceManager>;
