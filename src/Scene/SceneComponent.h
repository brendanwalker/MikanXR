#pragma once

#include "MikanComponent.h"
#include "Transform.h"
#include "IGlSceneRenderable.h"
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

	inline SceneComponentPtr getParentComponent() const
	{
		return m_parentComponent.lock();
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

	const glm::vec3 getWorldLocation() const;

	inline IGlSceneRenderablePtr getGlSceneRenderable() const
	{
		return m_sceneRenderable;
	}

	inline IGlSceneRenderableConstPtr getGlSceneRenderableConst() const
	{
		return m_sceneRenderable;
	}

	virtual void init() override;
	virtual void dispose() override;
	
	bool attachToComponent(SceneComponentPtr newParentComponent);
	enum class eDetachReason : int
	{
		selfDisposed,
		parentDisposed,
		detachFromParent,
		attachToNewParent
	};
	void detachFromParent(eDetachReason reason);

	virtual void setRelativeTransform(const GlmTransform& newRelativeXform);
	virtual void setWorldTransform(const glm::mat4& newWorldXform);

	enum class eTransformChangeType : int
	{
		recomputeWorldTransformAndPropogate,
		propogateWorldTransform,
	};
	//MulticastDelegate<void(SceneComponentPtr sceneComponent, eTransformChangeType changeType)> OnTranformChaged;

protected:
	void propogateWorldTransformChange(eTransformChangeType reason);

	GlmTransform m_relativeTransform;
	glm::mat4 m_worldTransform;
	SceneComponentWeakPtr m_parentComponent;
	SceneComponentList m_childComponents;
	IGlSceneRenderablePtr m_sceneRenderable;
};