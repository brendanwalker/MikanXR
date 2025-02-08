#include "IMkStaticMeshInstance.h"
#include "IMkCamera.h"
#include "IMkScene.h"
#include "IMkTriangulatedMesh.h"
#include "MkMaterialInstance.h"

class GlStaticMeshInstance : 
	public std::enable_shared_from_this<GlStaticMeshInstance>, 
	public IMkStaticMeshInstance
{
public:
	GlStaticMeshInstance() = delete;

	GlStaticMeshInstance::GlStaticMeshInstance(
		const std::string& name,
		IMkMeshConstPtr mesh)
		: m_name(name)
		, m_visible(true)
		, m_mesh(mesh)
		, m_materialInstance(std::make_shared<MkMaterialInstance>(mesh->getMaterialInstance()))
		, m_modelMatrix(glm::mat4(1.f))
		, m_normalMatrix(glm::mat4(1.f))
	{}

	virtual ~GlStaticMeshInstance()
	{
		assert(m_boundScene.lock() == nullptr);

		m_materialInstance = nullptr;
	}

	virtual const std::string& getName() const override
	{ 
		return m_name;
	}

	virtual void bindToScene(IMkScenePtr scene) override
	{
		removeFromBoundScene();

		m_boundScene = scene;
		scene->addInstance(getConstSelfPointer());
	}

	virtual void removeFromBoundScene() override
	{
		IMkScenePtr scene = m_boundScene.lock();
		if (scene != nullptr)
		{
			scene->removeInstance(getConstSelfPointer());
			m_boundScene.reset();
		}
	}

	virtual void setIsVisibleToCamera(const std::string& cameraName, bool bVisible) override
	{
		if (bVisible)
			m_visibileToCameras.insert(cameraName);
		else
			m_visibileToCameras.erase(cameraName);
	}

	virtual IMkMeshConstPtr getMesh() const override 
	{ 
		return m_mesh; 
	}

	// -- IGlSceneRenderable
	virtual IMkSceneRenderableConstPtr getConstSelfPointer() const override
	{
		return std::static_pointer_cast<const IMkSceneRenderable>(shared_from_this());
	}

	virtual bool getVisible() const override
	{
		return m_visible;
	}

	virtual void setVisible(bool bNewVisible) override
	{
		m_visible = bNewVisible;
	}

	virtual bool canCameraSee(IMkCameraConstPtr renderingCamera) const override
	{
		if (m_visibileToCameras.size() > 0)
		{
			const std::string& cameraName = renderingCamera->getName();

			return m_visibileToCameras.find(cameraName) != m_visibileToCameras.end();
		}

		return true;
	}

	virtual const glm::mat4& getModelMatrix() const override
	{
		return m_modelMatrix;
	}

	virtual const glm::mat4& getNormalMatrix() const override
	{
		return m_normalMatrix;
	}

	virtual void setModelMatrix(const glm::mat4& mat) override
	{
		m_modelMatrix = mat;

		// Normal matrix used in lighting calculations.
		// Preserves normals by undoing scale but preserving rotation.
		// See https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
		m_normalMatrix = glm::transpose(glm::inverse(glm::mat3(mat)));
	}

	virtual const MkMaterialInstanceConstPtr getMaterialInstanceConst() const override
	{
		return m_materialInstance;
	}

	virtual MkMaterialInstancePtr getMaterialInstance() const override
	{
		return m_materialInstance;
	}

	virtual void render() const override
	{
		m_mesh->drawElements();
	}

private:
	std::string m_name;
	bool m_visible = false;
	glm::mat4 m_modelMatrix;
	glm::mat4 m_normalMatrix;
	MkMaterialInstancePtr m_materialInstance = nullptr;
	IMkMeshConstPtr m_mesh = nullptr;
	IMkSceneWeakPtr m_boundScene;
	std::set<std::string> m_visibileToCameras;
};

IMkStaticMeshInstancePtr createMkStaticMeshInstance(
	const std::string& name,
	IMkMeshConstPtr mesh)
{
	return std::make_shared<GlStaticMeshInstance>(name, mesh);
}

