#pragma once

#include "IGLSceneRenderable.h"

#include <memory>
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class GlMaterial;
typedef std::shared_ptr<const GlMaterial> GlMaterialConstPtr;

class GlMaterialInstance;
typedef std::shared_ptr<GlMaterialInstance> GlMaterialInstancePtr;
typedef std::shared_ptr<const GlMaterialInstance> GlMaterialInstanceConstPtr;

class GlTriangulatedMesh;
typedef std::shared_ptr<const GlTriangulatedMesh> GlTriangulatedMeshConstPtr;

class GlStaticMeshInstance : public IGlSceneRenderable
{
public:
	GlStaticMeshInstance() = default;
	GlStaticMeshInstance(
		const std::string& name, 
		GlTriangulatedMeshConstPtr mesh, 
		GlMaterialConstPtr material);
	virtual ~GlStaticMeshInstance();

	const std::string& getName() const { return m_name; }

	void bindToScene(class GlScene* scene);
	void removeFromBoundScene();

	inline void setVisible(bool bNewVisible) { m_visible = bNewVisible; }
	inline void setModelMatrix(const glm::mat4& mat) { m_modelMatrix = mat; }

	inline GlTriangulatedMeshConstPtr getMesh() const { return m_mesh; }

	// -- IGlSceneRenderable
	virtual bool getVisible() const override;
	virtual const glm::mat4& getModelMatrix() const override;
	virtual const GlMaterialInstanceConstPtr getMaterialInstanceConst() const override;
	virtual GlMaterialInstancePtr getMaterialInstance() const override;
	virtual void render() const override;

private:
	std::string m_name;
	bool m_visible= false;
	glm::mat4 m_modelMatrix;
	GlMaterialInstancePtr m_materialInstance= nullptr;
	GlTriangulatedMeshConstPtr m_mesh= nullptr;
	class GlScene* m_boundScene= nullptr;
};