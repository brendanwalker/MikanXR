#pragma once

#include "GlTypesFwd.h"
#include <memory>

class GlCamera;
using GlCameraPtr = std::shared_ptr<GlCamera>;
using GlCameraConstPtr = std::shared_ptr<const GlCamera>;

class GlFrameBuffer;
using GlFrameBufferPtr = std::shared_ptr<GlFrameBuffer>;
using GlFrameBufferConstPtr = std::shared_ptr<const GlFrameBuffer>;

class GlLineRenderer;
using GlLineRendererSharedPtr = std::shared_ptr<GlLineRenderer>;
using GlLineRendererUniquePtr = std::unique_ptr<GlLineRenderer>;

class GlMaterial;
using GlMaterialPtr = std::shared_ptr<GlMaterial>;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class GlMaterialInstance;
using GlMaterialInstancePtr = std::shared_ptr<GlMaterialInstance>;
using GlMaterialInstanceConstPtr = std::shared_ptr<const GlMaterialInstance>;

class GlModelResourceManager;
using GlModelResourceManagerSharedPtr = std::shared_ptr<GlModelResourceManager>;
using GlModelResourceManagerUniquePtr = std::unique_ptr<GlModelResourceManager>;

class GlProgram;
using GlProgramPtr= std::shared_ptr<GlProgram>;
using GlProgramConstPtr= std::shared_ptr<const GlProgram>;

class GlProgramCode;
using GlProgramCodePtr = std::shared_ptr<GlProgramCode>;

class GlRenderModelResource;
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;
using GlRenderModelResourceWeakPtr = std::weak_ptr<GlRenderModelResource>;

class GlRmlUiRender;
using GlRmlUiRenderUniquePtr = std::unique_ptr<GlRmlUiRender>;

class GlScene;
using GlScenePtr = std::shared_ptr<GlScene>;
using GlSceneWeakPtr = std::weak_ptr<GlScene>;
using GlSceneConstPtr = std::shared_ptr<const GlScene>;

class GlShaderCache;
using GlShaderCacheUniquePtr = std::unique_ptr<GlShaderCache>;

class GlState;
class GlStateStack;
using GlStateStackSharedPtr = std::shared_ptr<GlStateStack>;
using GlStateStackUniquePtr = std::unique_ptr<GlStateStack>;

class GlStaticMeshInstance;
using GlStaticMeshInstancePtr = std::shared_ptr<GlStaticMeshInstance>;

class GlTextRenderer;
using GlTextRendererSharedPtr = std::shared_ptr<GlTextRenderer>;
using GlTextRendererUniquePtr = std::unique_ptr<GlTextRenderer>;

class GlTexture;
using GlTexturePtr = std::shared_ptr<GlTexture>;
using GlTextureConstPtr = std::shared_ptr<const GlTexture>;

class GlTriangulatedMesh;
using GlTriangulatedMeshPtr = std::shared_ptr<GlTriangulatedMesh>;

class GlViewport;
using GlViewportPtr = std::shared_ptr<GlViewport>;
using GlViewportWeakPtr = std::weak_ptr<GlViewport>;
using GlViewportConstPtr = std::shared_ptr<const GlViewport>;

class GlWireframeMesh;
using GlWireframeMeshPtr = std::shared_ptr<GlWireframeMesh>;

class IGlSceneRenderable;
using IGlSceneRenderablePtr= std::shared_ptr<IGlSceneRenderable>;
using IGlSceneRenderableConstPtr = std::shared_ptr<const IGlSceneRenderable>;
using IGlSceneRenderableConstWeakPtr = std::weak_ptr<const IGlSceneRenderable>;

struct GlVertexDefinition;