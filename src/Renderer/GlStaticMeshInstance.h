#pragma once

#include <string>
#include "glm/ext/matrix_float4x4.hpp"

class GlStaticMeshInstance
{
public:
	GlStaticMeshInstance() = default;
	GlStaticMeshInstance(
		const std::string& name, 
		const class GlTriangulatedMesh* mesh, 
		const class GlMaterial* material);
	virtual ~GlStaticMeshInstance();

	const std::string& getName() const { return m_name; }

	void bindToScene(class GlScene* scene);
	void removeFromBoundScene();

	inline bool getVisible() const { return m_visible; }
	inline void setVisible(bool bNewVisible) { m_visible = bNewVisible; }

	inline const glm::mat4& getModelMatrix() const { return m_modelMatrix; }
	inline void setModelMatrix(const glm::mat4& mat) { m_modelMatrix = mat; }

	inline class GlMaterialInstance* getMaterialInstance() const { return m_materialInstance; }
	inline const class GlTriangulatedMesh* getMesh() const { return m_mesh; }

	void render() const;

private:
	std::string m_name;
	bool m_visible= false;
	glm::mat4 m_modelMatrix;
	class GlMaterialInstance* m_materialInstance= nullptr;
	const class GlTriangulatedMesh* m_mesh= nullptr;
	class GlScene* m_boundScene= nullptr;
};