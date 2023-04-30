#pragma once

#include "RendererFwd.h"

class GlScopedMaterialBinding
{
public:
	GlScopedMaterialBinding() = default;
	GlScopedMaterialBinding(
		GlSceneConstPtr scene,
		GlCameraConstPtr camera,
		GlMaterialConstPtr material) 
		: m_boundScene(scene)
		, m_boundCamera(camera)
		, m_boundMaterial(material) 
	{}
	virtual ~GlScopedMaterialBinding();

	inline GlSceneConstPtr getBoundScene() const { return m_boundScene; }
	inline GlCameraConstPtr getBoundCamera() const { return m_boundCamera; }
	inline GlMaterialConstPtr getBoundMaterial() const { return m_boundMaterial; }
	inline operator bool() const { return m_boundMaterial != nullptr; }

private:
	GlSceneConstPtr m_boundScene;
	GlCameraConstPtr m_boundCamera;
	GlMaterialConstPtr m_boundMaterial;
};
