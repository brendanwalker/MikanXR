#pragma once

#include "MikanComponent.h"
#include "Transform.h"
#include "IGLSceneRenderable.h"
#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using SceneComponentList= std::vector<SceneComponentWeakPtr>;

class SceneComponent : public MikanComponent
{
public:
	SceneComponent(MikanObjectWeakPtr owner);

	inline SceneComponentWeakPtr getParentComponent() const
	{
		return m_parentComponent;
	}

	inline const SceneComponentList& getChildComponents() const
	{
		return m_childComponents;
	}

	inline const GlmTransform& getRelativeTransform() const
	{
		return m_relativeTransform;
	}

	inline const glm::mat4& getWorldTransform() const
	{
		return m_worldTransform;
	}

	inline IGlSceneRenderablePtr getGlSceneRenderable() const
	{
		return m_renderable;
	}

	inline IGlSceneRenderableConstPtr getGlSceneRenderableConst() const
	{
		return m_renderable;
	}

	virtual void init() override;
	virtual void dispose() override;
	
	void attachToComponent(SceneComponentWeakPtr newParentComponent);
	void detachFromParent();

	void setRelativeTransform(const GlmTransform& newRelativeXform);
	void setWorldTransform(const glm::mat4& newWorldXform);

protected:
	GlmTransform m_relativeTransform;
	glm::mat4 m_worldTransform;
	SceneComponentWeakPtr m_parentComponent;
	SceneComponentList m_childComponents;
	IGlSceneRenderablePtr m_renderable;
};