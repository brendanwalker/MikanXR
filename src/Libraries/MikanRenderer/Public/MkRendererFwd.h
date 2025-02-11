#pragma once

#include <memory>

class IMkFrameBuffer;
using IMkFrameBufferPtr = std::shared_ptr<IMkFrameBuffer>;
using IMkFrameBufferConstPtr = std::shared_ptr<const IMkFrameBuffer>;

class MkMaterial;
using MkMaterialPtr = std::shared_ptr<MkMaterial>;
using MkMaterialConstPtr = std::shared_ptr<const MkMaterial>;

class MkMaterialInstance;
using MkMaterialInstancePtr = std::shared_ptr<MkMaterialInstance>;
using MkMaterialInstanceConstPtr = std::shared_ptr<const MkMaterialInstance>;

class IMkVertexAttribute;
using IMkVertexAttributePtr = std::shared_ptr<IMkVertexAttribute>;
using IMkVertexAttributeConstPtr = std::shared_ptr<const IMkVertexAttribute>;

class IMkVertexDefinition;
using IMkVertexDefinitionPtr = std::shared_ptr<IMkVertexDefinition>;
using IMkVertexDefinitionConstPtr = std::shared_ptr<const IMkVertexDefinition>;

class IMkShader;
using IMkShaderPtr= std::shared_ptr<IMkShader>;
using IMkShaderConstPtr= std::shared_ptr<const IMkShader>;

class IMkShaderCode;
using IMkShaderCodePtr = std::shared_ptr<IMkShaderCode>;
using IMkShaderCodeConstPtr = std::shared_ptr<const IMkShaderCode>;

class MikanRenderModelResource;
using MikanRenderModelResourcePtr = std::shared_ptr<MikanRenderModelResource>;
using MikanRenderModelResourceWeakPtr = std::weak_ptr<MikanRenderModelResource>;

class IMkShaderCache;
using IMkShaderCachePtr = std::shared_ptr<IMkShaderCache>;

class IMkState;

class IMkStateModifier;
using IMkStateModifierPtr = std::shared_ptr<IMkStateModifier>;
using IMkStateModifierConstPtr = std::shared_ptr<const IMkStateModifier>;

class MkStateStack;
using MkStateStackSharedPtr = std::shared_ptr<MkStateStack>;
using MkStateStackUniquePtr = std::unique_ptr<MkStateStack>;

class IMkStaticMeshInstance;
using IMkStaticMeshInstancePtr = std::shared_ptr<IMkStaticMeshInstance>;
using IMkStaticMeshInstanceConstPtr = std::shared_ptr<const IMkStaticMeshInstance>;

class IMkTexture;
using IMkTexturePtr = std::shared_ptr<IMkTexture>;
using IMkTextureConstPtr = std::shared_ptr<const IMkTexture>;

class IMkLineRenderer;
using IMkLineRendererPtr = std::shared_ptr<IMkLineRenderer>;
using IMkLineRendererConstPtr = std::shared_ptr<const IMkLineRenderer>;

class IMkTextRenderer;
using IMkTextRendererPtr = std::shared_ptr<IMkTextRenderer>;
using IMkTextRendererConstPtr = std::shared_ptr<const IMkTextRenderer>;

class IMkTextureCache;
using IMkTextureCachePtr = std::shared_ptr<IMkTextureCache>;

class IMkTriangulatedMesh;
using IMkTriangulatedMeshPtr = std::shared_ptr<IMkTriangulatedMesh>;
using IMkTriangulatedMeshConstPtr = std::shared_ptr<const IMkTriangulatedMesh>;

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

class IMkScene;
using IMkScenePtr = std::shared_ptr<IMkScene>;
using IMkSceneWeakPtr = std::weak_ptr<IMkScene>;
using IMkSceneConstPtr = std::shared_ptr<const IMkScene>;

class IMkSceneRenderable;
using IMkSceneRenderablePtr= std::shared_ptr<IMkSceneRenderable>;
using IMkSceneRenderableConstPtr = std::shared_ptr<const IMkSceneRenderable>;
using IMkSceneRenderableConstWeakPtr = std::weak_ptr<const IMkSceneRenderable>;

class IMkVertexAttribute;
class IMkVertexDefinition;