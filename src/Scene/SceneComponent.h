#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "ObjectSystemConfigFwd.h"
#include "Transform.h"
#include "IGlSceneRenderable.h"
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

	static const std::string k_relativeQuatPropertyId;
	const MikanQuatf getRelativeQuat() const { return m_relativeTransform.rotation; }
	void setRelativeQuat(const MikanQuatf& quat);

	static const std::string k_relativeTranslationPropertyId;
	const MikanVector3f getRelativeTranslation() const { return m_relativeTransform.translation; }
	void setRelativeTransition(const MikanVector3f& translation);

private:
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

	virtual void setDefinition(MikanComponentDefinitionPtr config) override;
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
	void setRelativeOrientation(const glm::quat& quat);
	void setRelativeScale(const glm::vec3& scale);

	void setWorldTransform(const glm::mat4& newWorldXform);

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
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
	IGlSceneRenderablePtr m_sceneRenderable;
};