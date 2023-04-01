#pragma once

#include "MikanComponent.h"
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class MikanSceneComponent;
typedef std::weak_ptr<MikanSceneComponent> MikanSceneComponentWeakPtr;
typedef std::shared_ptr<MikanSceneComponent> MikanSceneComponentPtr;

typedef std::vector<MikanSceneComponentWeakPtr> MikanSceneComponentList;

class MikanSceneComponent : public MikanComponent
{
public:
	MikanSceneComponent(MikanObjectWeakPtr owner);

	inline MikanSceneComponentWeakPtr getParentComponent() const
	{
		return m_parentComponent;
	}

	inline const MikanSceneComponentList& getChildComponents() const
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

	virtual void init() override;
	virtual void dispose() override;

	void attachToComponent(MikanSceneComponentWeakPtr newParentComponent);
	void detachFromParent();

	void setRelativeTransform(const GlmTransform& newRelativeXform);
	void setWorldTransform(const glm::mat4& newWorldXform);

protected:
	GlmTransform m_relativeTransform;
	glm::mat4 m_worldTransform;
	MikanSceneComponentWeakPtr m_parentComponent;
	MikanSceneComponentList m_childComponents;
};