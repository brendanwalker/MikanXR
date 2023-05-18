#pragma once

#include <string>
#include <set>

#include "RendererFwd.h"

using UniformNameSet = std::set<std::string>;

class GlScopedMaterialBinding
{
public:
	GlScopedMaterialBinding() = default;
	GlScopedMaterialBinding(
		GlSceneConstPtr scene,
		GlCameraConstPtr camera,
		GlMaterialConstPtr material,
		UniformNameSet unboundUniformNames,
		bool bMaterialFailure)
		: m_boundScene(scene)
		, m_boundCamera(camera)
		, m_boundMaterial(material) 
		, m_unboundUniformNames(unboundUniformNames)
		, m_bMaterialFailure(bMaterialFailure)
	{}
	virtual ~GlScopedMaterialBinding();

	inline GlSceneConstPtr getBoundScene() const { return m_boundScene; }
	inline GlCameraConstPtr getBoundCamera() const { return m_boundCamera; }
	inline GlMaterialConstPtr getBoundMaterial() const { return m_boundMaterial; }
	inline const UniformNameSet& getUnboundUniforms() const { return m_unboundUniformNames; }
	inline operator bool() const { return !m_bMaterialFailure; }

private:
	GlSceneConstPtr m_boundScene;
	GlCameraConstPtr m_boundCamera;
	GlMaterialConstPtr m_boundMaterial;
	UniformNameSet m_unboundUniformNames;
	bool m_bMaterialFailure;
};
