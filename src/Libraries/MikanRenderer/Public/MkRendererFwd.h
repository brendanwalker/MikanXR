#pragma once

#include <memory>

class GlFrameBuffer;
using GlFrameBufferPtr = std::shared_ptr<GlFrameBuffer>;
using GlFrameBufferConstPtr = std::shared_ptr<const GlFrameBuffer>;

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

class GlShaderCache;
using GlShaderCacheUniquePtr = std::unique_ptr<GlShaderCache>;

class GlState;
class GlStateStack;
using GlStateStackSharedPtr = std::shared_ptr<GlStateStack>;
using GlStateStackUniquePtr = std::unique_ptr<GlStateStack>;

class GlStaticMeshInstance;
using GlStaticMeshInstancePtr = std::shared_ptr<GlStaticMeshInstance>;
using GlStaticMeshInstanceConstPtr = std::shared_ptr<const GlStaticMeshInstance>;

class IMkTexture;
using IMkTexturePtr = std::shared_ptr<IMkTexture>;
using IMkTextureConstPtr = std::shared_ptr<const IMkTexture>;

class IMkLineRenderer;
using IMkLineRendererPtr = std::shared_ptr<IMkLineRenderer>;
using IMkLineRendererConstPtr = std::shared_ptr<const IMkLineRenderer>;

class GlTextureCache;
using GlTextureCacheUniquePtr = std::unique_ptr<GlTextureCache>;

class GlTriangulatedMesh;
using GlTriangulatedMeshPtr = std::shared_ptr<GlTriangulatedMesh>;
using GlTriangulatedMeshConstPtr = std::shared_ptr<const GlTriangulatedMesh>;

class IMkViewport;
using IMkViewportPtr = std::shared_ptr<IMkViewport>;
using IMkViewportWeakPtr = std::weak_ptr<IMkViewport>;
using IMkViewportConstPtr = std::shared_ptr<const IMkViewport>;

class IMkWireframeMesh;
using IMkWireframeMeshPtr = std::shared_ptr<IMkWireframeMesh>;

class IMkBindableObject;
using IMkBindableObjectPtr = std::shared_ptr<IMkBindableObject>;
using IMkBindableObjectConstPtr = std::shared_ptr<const IMkBindableObject>;

class IMkCamera;
using IMkCameraPtr = std::shared_ptr<IMkCamera>;
using IMkCameraConstPtr = std::shared_ptr<const IMkCamera>;

class IMkWindow;
using IMkWindowPtr = std::shared_ptr<IMkWindow>;
using IMkWindowConstPtr = std::shared_ptr<const IMkWindow>;

class IMkMesh;
using IMkMeshPtr = std::shared_ptr<IMkMesh>;
using IMkMeshConstPtr = std::shared_ptr<const IMkMesh>;

class IMkSceneRenderable;
using IMkSceneRenderablePtr= std::shared_ptr<IMkSceneRenderable>;
using IMkSceneRenderableConstPtr = std::shared_ptr<const IMkSceneRenderable>;
using IMkSceneRenderableConstWeakPtr = std::weak_ptr<const IMkSceneRenderable>;

class GlVertexAttribute;
class GlVertexDefinition;