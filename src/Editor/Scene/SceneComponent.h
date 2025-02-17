#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "Transform.h"
#include "IMkSceneRenderable.h"
#include "ComponentFwd.h"
#include "ObjectFwd.h"
#include "SceneFwd.h"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using SceneComponentList= std::vector<SceneComponentWeakPtr>;

class SceneComponentDefinition : public MikanComponentDefinition
{
public:
	SceneComponentDefinition();
	SceneComponentDefinition(const std::string& componentName, const MikanTransform& xform);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	const glm::mat4 getRelativeMat4() const;
	void setRelativeMat4(const glm::mat4& xform);

	const GlmTransform getRelativeTransform() const;
	void setRelativeTransform(const GlmTransform& transform);

	static const std::string k_relativeScalePropertyId;
	const MikanVector3f getRelativeScale() const { return m_relativeTransform.scale; }
	void setRelativeScale(const MikanVector3f& scale);

	static const std::string k_relativeRotationPropertyId;
	const MikanQuatf getRelativeRotation() const { return m_relativeTransform.rotation; }
	void setRelativeRotation(const MikanQuatf& quat);

	static const std::string k_relativePositionPropertyId;
	const MikanVector3f getRelativePosition() const { return m_relativeTransform.position; }
	void setRelativePosition(const MikanVector3f& translation);

protected:
	MikanTransform m_relativeTransform;
};

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

	inline IMkSceneRenderablePtr getGlSceneRenderable() const
	{
		return m_sceneRenderable;
	}

	inline IMkSceneRenderableConstPtr getGlSceneRenderableConst() const
	{
		return m_sceneRenderable;
	}

	virtual void init() override;
	virtual void dispose() override;

	virtual void setDefinition(MikanComponentDefinitionPtr config) override;
	inline SceneComponentDefinitionConstPtr getSceneComponentDefinitionConst() const
	{
		return std::static_pointer_cast<const SceneComponentDefinition>(m_definition);
	}
	inline SceneComponentDefinitionPtr getSceneComponentDefinition() { 
		return std::static_pointer_cast<SceneComponentDefinition>(m_definition); 
	}
	
	bool attachToComponent(SceneComponentPtr newParentComponent);
	enum class eDetachReason : int
	{
		selfDisposed,
		parentDisposed,
		detachFromParent,
		attachToNewParent
	};
	void detachFromParent(eDetachReason reason);

	void setRelativeTransform(const GlmTransform& newRelativeXform);
	void setRelativePosition(const glm::vec3& position);
	void setRelativeRotation(const glm::quat& quat);
	void setRelativeScale(const glm::vec3& scale);

	inline const GlmTransform& getRelativeTransform() const { return m_relativeTransform; }
	inline const glm::vec3& getRelativePosition() const { return m_relativeTransform.getPosition(); }
	inline const glm::quat& getRelativeRotation() const { return m_relativeTransform.getRotation(); }
	inline const glm::vec3& getRelativeScale() const { return m_relativeTransform.getScale(); }

	void setWorldTransform(const glm::mat4& newWorldXform);
	inline const glm::mat4& getWorldTransform() const { return m_worldTransform; }
	const glm::vec3 getWorldLocation() const;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

protected:
	enum class eTransformChangeType : int
	{
		recomputeWorldTransformAndPropogate,
		propogateWorldTransform,
	};

	void propogateWorldTransformChange(eTransformChangeType reason);

	GlmTransform m_relativeTransform;
	glm::mat4 m_worldTransform;
	SceneComponentWeakPtr m_parentComponent;
	SceneComponentList m_childComponents;
	IMkSceneRenderablePtr m_sceneRenderable;
};