#pragma once

#include <memory>

class GlCamera;
using GlCameraPtr = std::shared_ptr<GlCamera>;
using GlCameraConstPtr = std::shared_ptr<const GlCamera>;

class GlMaterial;
using GlMaterialPtr = std::shared_ptr<GlMaterial>;
using GlMaterialConstPtr = std::shared_ptr<const GlMaterial>;

class GlMaterialInstance;
using GlMaterialInstancePtr = std::shared_ptr<GlMaterialInstance>;
using GlMaterialInstanceConstPtr = std::shared_ptr<const GlMaterialInstance>;

class GlProgram;
using GlProgramPtr= std::shared_ptr<GlProgram>;
using GlProgramConstPtr= std::shared_ptr<const GlProgram>;

class GlProgramCode;
using GlProgramCodePtr = std::shared_ptr<GlProgramCode>;

class GlRenderModelResource;
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;
using GlRenderModelResourceWeakPtr = std::weak_ptr<GlRenderModelResource>;

class GlScene;
using GlScenePtr = std::shared_ptr<GlScene>;
using GlSceneConstPtr = std::shared_ptr<const GlScene>;

class GlStaticMeshInstance;
using GlStaticMeshInstancePtr = std::shared_ptr<GlStaticMeshInstance>;

class GlTexture;
using GlTexturePtr = std::shared_ptr<GlTexture>;

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

class IGlLineRenderable;
using IGlLineRenderablePtr = std::shared_ptr<IGlLineRenderable>;
using IGlLineRenderableConstPtr = std::shared_ptr<const IGlLineRenderable>;
using IGlLineRenderableConstWeakPtr = std::weak_ptr<const IGlLineRenderable>;