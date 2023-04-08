#pragma once

#include "IGLSceneRenderable.h"
#include "IGlMesh.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class GlMaterial;
typedef std::shared_ptr<const GlMaterial> GlMaterialConstPtr;

class GlMaterialInstance;
typedef std::shared_ptr<GlMaterialInstance> GlMaterialInstancePtr;
typedef std::shared_ptr<const GlMaterialInstance> GlMaterialInstanceConstPtr;

class GlStaticMeshInstance : public IGlSceneRenderable
{
public:
	GlStaticMeshInstance() = default;
	GlStaticMeshInstance(
		const std::string& name, 
		IGlMeshConstPtr mesh, 
		GlMaterialConstPtr material);
	GlStaticMeshInstance(
		const std::string& name,
		IGlMeshConstPtr mesh,
		GlMaterialInstancePtr materialInstance);
	virtual ~GlStaticMeshInstance();

	const std::string& getName() const { return m_name; }

	void bindToScene(class GlScene* scene);
	void removeFromBoundScene();

	inline IGlMeshConstPtr getMesh() const { return m_mesh; }

	// -- IGlSceneRenderable
	virtual bool getVisible() const override;
	virtual void setVisible(bool bNewVisible) override;
	virtual const glm::mat4& getModelMatrix() const override;
	virtual void setModelMatrix(const glm::mat4& mat) override;
	virtual const GlMaterialInstanceConstPtr getMaterialInstanceConst() const override;
	virtual GlMaterialInstancePtr getMaterialInstance() const override;
	virtual void render() const override;

private:
	std::string m_name;
	bool m_visible= false;
	glm::mat4 m_modelMatrix;
	GlMaterialInstancePtr m_materialInstance= nullptr;
	IGlMeshConstPtr m_mesh= nullptr;
	class GlScene* m_boundScene= nullptr;
};
typedef std::shared_ptr<GlStaticMeshInstance> GlStaticMeshInstancePtr;