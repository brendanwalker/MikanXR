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

class GlTexture;
using GlTexturePtr = std::shared_ptr<GlTexture>;
using GlTextureConstPtr = std::shared_ptr<const GlTexture>;

class GlTextureCache;
using GlTextureCacheUniquePtr = std::unique_ptr<GlTextureCache>;

class GlTriangulatedMesh;
using GlTriangulatedMeshPtr = std::shared_ptr<GlTriangulatedMesh>;
using GlTriangulatedMeshConstPtr = std::shared_ptr<const GlTriangulatedMesh>;

class GlWireframeMesh;
using GlWireframeMeshPtr = std::shared_ptr<GlWireframeMesh>;

class IMkCamera;
using IMkCameraPtr = std::shared_ptr<IMkCamera>;
using IMkCameraConstPtr = std::shared_ptr<const IMkCamera>;

class IGlMesh;
using IGlMeshPtr = std::shared_ptr<IGlMesh>;
using IGlMeshConstPtr = std::shared_ptr<const IGlMesh>;

class IMkSceneRenderable;
using IMkSceneRenderablePtr= std::shared_ptr<IMkSceneRenderable>;
using IMkSceneRenderableConstPtr = std::shared_ptr<const IMkSceneRenderable>;
using IMkSceneRenderableConstWeakPtr = std::weak_ptr<const IMkSceneRenderable>;

class GlVertexAttribute;
class GlVertexDefinition;