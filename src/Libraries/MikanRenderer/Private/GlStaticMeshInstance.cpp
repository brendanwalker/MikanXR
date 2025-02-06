#include "GlStaticMeshInstance.h"
#include "IMkCamera.h"
//#include "GlScene.h"
#include "GlTriangulatedMesh.h"
#include "GlMaterialInstance.h"

GlStaticMeshInstance::GlStaticMeshInstance(
	const std::string& name, 
	IMkMeshConstPtr mesh)
	: m_name(name)
	, m_visible(true)
	, m_mesh(mesh)
	, m_materialInstance(std::make_shared<GlMaterialInstance>(mesh->getMaterialInstance()))
	, m_modelMatrix(glm::mat4(1.f))
	, m_normalMatrix(glm::mat4(1.f))
{
}

GlStaticMeshInstance::~GlStaticMeshInstance()
{
	assert(m_boundScene.lock() == nullptr);

	m_materialInstance= nullptr;
}

void GlStaticMeshInstance::bindToScene(GlScenePtr scene)
{
	removeFromBoundScene();

	m_boundScene= scene;
	scene->addInstance(getConstSelfPointer());
}

void GlStaticMeshInstance::removeFromBoundScene()
{
	GlScenePtr scene= m_boundScene.lock();
	if (scene != nullptr)
	{
		scene->removeInstance(getConstSelfPointer());
		m_boundScene.reset();
	}
}

void GlStaticMeshInstance::setIsVisibleToCamera(const std::string& cameraName, bool bVisible)
{
	if (bVisible)
		m_visibileToCameras.insert(cameraName);
	else
		m_visibileToCameras.erase(cameraName);
}

// -- IGlSceneRenderable
IMkSceneRenderableConstPtr GlStaticMeshInstance::getConstSelfPointer() const
{
	return std::static_pointer_cast<const IMkSceneRenderable>(shared_from_this());
}

bool GlStaticMeshInstance::getVisible() const 
{ 
	return m_visible; 
}

void GlStaticMeshInstance::setVisible(bool bNewVisible) 
{ 
	m_visible = bNewVisible; 
}

bool GlStaticMeshInstance::canCameraSee(IMkCameraConstPtr renderingCamera) const
{
	if (m_visibileToCameras.size() > 0)
	{
		const std::string& cameraName = renderingCamera->getName();

		return m_visibileToCameras.find(cameraName) != m_visibileToCameras.end();
	}

	return true;
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
