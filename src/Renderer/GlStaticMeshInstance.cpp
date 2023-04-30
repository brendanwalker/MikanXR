#include "GlStaticMeshInstance.h"
#include "GlScene.h"
#include "GlTriangulatedMesh.h"
#include "GlMaterialInstance.h"

GlStaticMeshInstance::GlStaticMeshInstance(
	const std::string& name, 
	IGlMeshConstPtr mesh, 
	GlMaterialConstPtr material)
	: m_name(name)
	, m_visible(true)
	, m_mesh(mesh)
	, m_materialInstance(std::make_shared<GlMaterialInstance>(material))
	, m_modelMatrix(glm::mat4(1.f))
	, m_normalMatrix(glm::mat4(1.f))
{

}

GlStaticMeshInstance::GlStaticMeshInstance(
	const std::string& name,
	IGlMeshConstPtr mesh,
	GlMaterialInstancePtr materialInstance)
	: m_name(name)
	, m_visible(true)
	, m_mesh(mesh)
	, m_materialInstance(materialInstance)
	, m_modelMatrix(glm::mat4(1.f))
	, m_normalMatrix(glm::mat4(1.f))
{

}

GlStaticMeshInstance::~GlStaticMeshInstance()
{
	assert(m_boundScene == nullptr);

	m_materialInstance= nullptr;
}

void GlStaticMeshInstance::bindToScene(GlScene* scene)
{
	removeFromBoundScene();

	m_boundScene= scene;
	m_boundScene->addInstance(IGlSceneRenderableConstPtr(this));
}

void GlStaticMeshInstance::removeFromBoundScene()
{
	if (m_boundScene != nullptr)
	{
		m_boundScene->removeInstance(IGlSceneRenderableConstPtr(this));
		m_boundScene = nullptr;
	}
}

// -- IGlSceneRenderable
bool GlStaticMeshInstance::getVisible() const 
{ 
	return m_visible; 
}

void GlStaticMeshInstance::setVisible(bool bNewVisible) 
{ 
	m_visible = bNewVisible; 
}

const glm::mat4& GlStaticMeshInstance::getModelMatrix() const 
{
	return m_modelMatrix; 
}

const glm::mat4& GlStaticMeshInstance::getNormalMatrix() const
{
	return m_normalMatrix;
}

void GlStaticMeshInstance::setModelMatrix(const glm::mat4& mat) 
{ 
	m_modelMatrix = mat;

	// Normal matrix used in lighting calculations.
	// Preserves normals by undoing scale but preserving rotation.
	// See https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
	m_normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat)));
}

const GlMaterialInstanceConstPtr GlStaticMeshInstance::getMaterialInstanceConst() const 
{ 
	return m_materialInstance; 
}

GlMaterialInstancePtr GlStaticMeshInstance::getMaterialInstance() const 
{ 
	return m_materialInstance; 
}

void GlStaticMeshInstance::render() const
{
	m_mesh->drawElements();
}
