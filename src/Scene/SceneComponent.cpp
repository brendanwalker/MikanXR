#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

// -- ModelStencilConfig -----
const std::string SceneComponentDefinition::k_relativeScalePropertyId = "relative_scale";
const std::string SceneComponentDefinition::k_relativeQuatPropertyId = "relative_quat";
const std::string SceneComponentDefinition::k_relativeTranslationPropertyId = "relative_transition";

SceneComponentDefinition::SceneComponentDefinition()
	: MikanComponentDefinition()
{
	m_relativeTransform.scale = {1.f, 1.f, 1.f};
	m_relativeTransform.rotation= {1.f, 0.f, 0.f, 0.f};
	m_relativeTransform.translation= {0.f, 0.f, 0.f};
}

SceneComponentDefinition::SceneComponentDefinition(
	const std::string& componentName,
	const MikanTransform& xform)
	: MikanComponentDefinition(componentName)
{
	m_relativeTransform= xform;
}

configuru::Config SceneComponentDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	writeVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.scale);
	writeQuatf(pt, k_relativeQuatPropertyId.c_str(), m_relativeTransform.rotation);
	writeVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.translation);

	return pt;
}

void SceneComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_relativeTransform.scale = {1.f, 1.f, 1.f};
	m_relativeTransform.rotation = {1.f, 0.f, 0.f, 0.f};
	m_relativeTransform.translation = {0.f, 0.f, 0.f};

	readVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.scale);
	readQuatf(pt, k_relativeQuatPropertyId.c_str(), m_relativeTransform.rotation);
	readVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.translation);
}

const glm::mat4 SceneComponentDefinition::getRelativeMat4() const
{
	return getRelativeTransform().getMat4();
}

void SceneComponentDefinition::setRelativeMat4(const glm::mat4& mat4)
{
	setRelativeTransform(GlmTransform(mat4));
}

const GlmTransform SceneComponentDefinition::getRelativeTransform() const
{
	return GlmTransform(
		MikanVector3f_to_glm_vec3(m_relativeTransform.translation),
		MikanQuatf_to_glm_quat(m_relativeTransform.rotation),
		MikanVector3f_to_glm_vec3(m_relativeTransform.scale));
}

void SceneComponentDefinition::setRelativeTransform(const GlmTransform& transform)
{
	glm::mat4 xform = transform.getMat4();

	m_relativeTransform.translation = glm_vec3_to_MikanVector3f(transform.getPosition());
	m_relativeTransform.rotation = glm_quat_to_MikanQuatf(transform.getOrientation());
	m_relativeTransform.scale = glm_vec3_to_MikanVector3f(transform.getScale());

	markDirty(ConfigPropertyChangeSet()
			  .addPropertyName(k_relativeScalePropertyId)
			  .addPropertyName(k_relativeQuatPropertyId)
			  .addPropertyName(k_relativeTranslationPropertyId));
}

void SceneComponentDefinition::setRelativeScale(const MikanVector3f& scale)
{
	m_relativeTransform.scale = scale;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativeScalePropertyId));
}

void SceneComponentDefinition::setRelativeQuat(const MikanQuatf& quat)
{
	m_relativeTransform.rotation = quat;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativeQuatPropertyId));
}

void SceneComponentDefinition::setRelativeTransition(const MikanVector3f& translation)
{
	m_relativeTransform.translation = translation;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativeTranslationPropertyId));
}

// -- Scene Component -----
SceneComponent::SceneComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_relativeTransform()
	, m_worldTransform(glm::mat4(1.f))
{
}

void SceneComponent::init()
{
	MikanComponent::init();
}

void SceneComponent::dispose()
{
	// Detach all children from this scene component first
	while (m_childComponents.size() > 0)
	{
		SceneComponentPtr childComponent= m_childComponents[0].lock();

		if (childComponent)
		{
			childComponent->detachFromParent(eDetachReason::parentDisposed);
		}
	}

	// Detach from our parent next
	if (m_parentComponent.lock())
	{
		detachFromParent(eDetachReason::selfDisposed);
	}

	MikanComponent::dispose();
}

void SceneComponent::setDefinition(MikanComponentDefinitionPtr config)
{
	MikanComponent::setDefinition(config);

	auto sceneComponentConfigPtr= std::static_pointer_cast<SceneComponentDefinition>(config);

	// Initially the model component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform = sceneComponentConfigPtr->getRelativeMat4();
	m_relativeTransform = GlmTransform(m_worldTransform);
}

bool SceneComponent::attachToComponent(SceneComponentPtr newParentComponent)
{
	SceneComponentPtr oldParentComponent= m_parentComponent.lock();

	if (!newParentComponent || newParentComponent == oldParentComponent)
		return false;

	// Detach from old parent
	if (oldParentComponent)
	{
		detachFromParent(eDetachReason::detachFromParent);
	}

	// Attach to new parent
	SceneComponentList& parentChildList= newParentComponent->m_childComponents;

	parentChildList.push_back(getSelfWeakPtr<SceneComponent>());
	m_parentComponent = newParentComponent;

	// Refresh world transform
	setRelativeTransform(m_relativeTransform);

	return true;
}

void SceneComponent::detachFromParent(eDetachReason reason)
{
	// Remove ourselves from our current parent's child list
	SceneComponentPtr parent= m_parentComponent.lock();
	if (parent != nullptr)
	{
		SceneComponentList& parentChildList= parent->m_childComponents;

		for (auto it = parentChildList.begin(); it != parentChildList.end(); ++it)
		{
			SceneComponentPtr sceneComponent= it->lock(); 

			if (sceneComponent.get() == this)
			{
				parentChildList.erase(it);
				break;
			}
		}
	}

	// Forget about our parent
	m_parentComponent = SceneComponentPtr();

	// World transform is now the same as relative transform
	m_worldTransform= m_relativeTransform.getMat4();

	// Only bother propagating our new transform to our children 
	// if we just doing a simple detach from our current parent
	if (reason == eDetachReason::detachFromParent)
	{
		propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
	}
}

void SceneComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	// Set the new relative transform directly
	m_relativeTransform= newRelativeXform;
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bIsInitialized)
		getSceneComponentDefinition()->setRelativeTransform(newRelativeXform);
}

void SceneComponent::setRelativePosition(const glm::vec3& position)
{
	m_relativeTransform.setPosition(position);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bIsInitialized)
		getSceneComponentDefinition()->setRelativeTransition(glm_vec3_to_MikanVector3f(position));
}

void SceneComponent::setRelativeOrientation(const glm::quat& quat)
{
	m_relativeTransform.setOrientation(quat);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bIsInitialized)
		getSceneComponentDefinition()->setRelativeQuat(glm_quat_to_MikanQuatf(quat));
}

void SceneComponent::setRelativeScale(const glm::vec3& scale)
{
	m_relativeTransform.setScale(scale);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bIsInitialized)
		getSceneComponentDefinition()->setRelativeScale(glm_vec3_to_MikanVector3f(scale));
}

void SceneComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	// Set the new world transform directly
	m_worldTransform= newWorldXform;

	// Subtract our new world transform from out parent to compute new relative transform
	glm::mat4 invParentXform= glm::mat4(1.f);
	SceneComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		invParentXform= glm::inverse(parent->getWorldTransform());
	}
	const glm::mat4 relativeXform= glm_composite_xform(m_worldTransform, invParentXform);
	m_relativeTransform= GlmTransform(relativeXform);

	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);

	if (m_bIsInitialized)
		getSceneComponentDefinition()->setRelativeTransform(m_relativeTransform);
}

const glm::vec3 SceneComponent::getWorldLocation() const
{
	return glm_mat4_get_position(m_worldTransform);
}

void SceneComponent::propogateWorldTransformChange(eTransformChangeType reason)
{
	// Recompute our world transform, if requested
	if (reason == eTransformChangeType::recomputeWorldTransformAndPropogate)
	{
		// Concat our transform to our parent's world transform
		SceneComponentPtr parent = m_parentComponent.lock();
		if (parent != nullptr)
		{
			m_worldTransform = glm_composite_xform(m_relativeTransform.getMat4(), parent->getWorldTransform());
		}
		else
		{
			m_worldTransform = m_relativeTransform.getMat4();
		}
	}

	// Update the world transform on the attached IGlSceneRenderable
	if (m_sceneRenderable != nullptr)
	{
		m_sceneRenderable->setModelMatrix(m_worldTransform);
	}

	// Propagate our updated world transform to our children
	for (SceneComponentWeakPtr childComponentWeakPtr : m_childComponents)
	{
		SceneComponentPtr childComponent= childComponentWeakPtr.lock();

		if (childComponent != nullptr)
		{
			// Ask all children to recompute their world transform
			childComponent->propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
		}
	}
}