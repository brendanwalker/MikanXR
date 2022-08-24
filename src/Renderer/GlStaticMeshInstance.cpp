#include "GlStaticMeshInstance.h"
#include "GlScene.h"
#include "GlTriangulatedMesh.h"
#include "GlMaterialInstance.h"

GlStaticMeshInstance::GlStaticMeshInstance(
	const std::string& name, 
	const GlTriangulatedMesh* mesh, 
	const GlMaterial* material)
{
	m_name = name;
	m_visible = true;
	m_mesh = mesh;
	m_materialInstance = new GlMaterialInstance(material);
	m_modelMatrix= glm::mat4(1.f);
}

GlStaticMeshInstance::~GlStaticMeshInstance()
{
	assert(m_boundScene == nullptr);

	if (m_materialInstance != nullptr)
	{
		delete m_materialInstance;
	}
}

void GlStaticMeshInstance::bindToScene(GlScene* scene)
{
	removeFromBoundScene();

	m_boundScene= scene;
	m_boundScene->addInstance(this);
}

void GlStaticMeshInstance::removeFromBoundScene()
{
	if (m_boundScene != nullptr)
	{
		m_boundScene->removeInstance(this);
		m_boundScene = nullptr;
	}
}

void GlStaticMeshInstance::render() const
{
	m_mesh->drawElements();
}
