#include "TransformComponent.h"
#include "MikanObject.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>
#include <RmlUi/Core/Vector3.h>

// -- ModelStencilConfig -----
const std::string TransformComponentDefinition::k_relativeScalePropertyId = "relative_scale";
const std::string TransformComponentDefinition::k_relativeRotationPropertyId = "relative_rotation";
const std::string TransformComponentDefinition::k_relativePositionPropertyId = "relative_position";

TransformComponentDefinition::TransformComponentDefinition()
	: MikanComponentDefinition()
{
	m_relativeTransform.scale = {1.f, 1.f, 1.f};
	m_relativeTransform.rotation= {1.f, 0.f, 0.f, 0.f};
	m_relativeTransform.position= {0.f, 0.f, 0.f};
}

TransformComponentDefinition::TransformComponentDefinition(
	const std::string& componentName,
	const MikanTransform& xform)
	: MikanComponentDefinition(componentName)
{
	m_relativeTransform= xform;
}

configuru::Config TransformComponentDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	writeVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.scale);
	writeQuatf(pt, k_relativeRotationPropertyId.c_str(), m_relativeTransform.rotation);
	writeVector3f(pt, k_relativePositionPropertyId.c_str(), m_relativeTransform.position);

	return pt;
}

void TransformComponentDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_relativeTransform.scale = {1.f, 1.f, 1.f};
	m_relativeTransform.rotation = {1.f, 0.f, 0.f, 0.f};
	m_relativeTransform.position = {0.f, 0.f, 0.f};

	readVector3f(pt, k_relativeScalePropertyId.c_str(), m_relativeTransform.scale);
	readQuatf(pt, k_relativeRotationPropertyId.c_str(), m_relativeTransform.rotation);
	readVector3f(pt, k_relativePositionPropertyId.c_str(), m_relativeTransform.position);
}

const glm::mat4 TransformComponentDefinition::getRelativeMat4() const
{
	return getRelativeTransform().getMat4();
}

void TransformComponentDefinition::setRelativeMat4(const glm::mat4& mat4)
{
	setRelativeTransform(GlmTransform(mat4));
}

const GlmTransform TransformComponentDefinition::getRelativeTransform() const
{
	return GlmTransform(
		MikanVector3f_to_glm_vec3(m_relativeTransform.position),
		MikanQuatf_to_glm_quat(m_relativeTransform.rotation),
		MikanVector3f_to_glm_vec3(m_relativeTransform.scale));
}

void TransformComponentDefinition::setRelativeTransform(const GlmTransform& transform)
{
	glm::mat4 xform = transform.getMat4();

	m_relativeTransform.position = glm_vec3_to_MikanVector3f(transform.getPosition());
	m_relativeTransform.rotation = glm_quat_to_MikanQuatf(transform.getRotation());
	m_relativeTransform.scale = glm_vec3_to_MikanVector3f(transform.getScale());

	markDirty(ConfigPropertyChangeSet()
			  .addPropertyName(k_relativeScalePropertyId)
			  .addPropertyName(k_relativeRotationPropertyId)
			  .addPropertyName(k_relativePositionPropertyId));
}

void TransformComponentDefinition::setRelativeScale(const MikanVector3f& scale)
{
	m_relativeTransform.scale = scale;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativeScalePropertyId));
}

void TransformComponentDefinition::setRelativeRotation(const MikanQuatf& quat)
{
	m_relativeTransform.rotation = quat;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativeRotationPropertyId));
}

void TransformComponentDefinition::setRelativePosition(const MikanVector3f& translation)
{
	m_relativeTransform.position = translation;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_relativePositionPropertyId));
}

// -- Scene Component -----
TransformComponent::TransformComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_relativeTransform()
	, m_worldTransform(glm::mat4(1.f))
{
}

void TransformComponent::init()
{
	MikanComponent::init();
}

void TransformComponent::dispose()
{
	// Detach all children from this scene component first
	while (m_childComponents.size() > 0)
	{
		TransformComponentPtr childComponent= m_childComponents[0].lock();

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

void TransformComponent::setDefinition(MikanComponentDefinitionPtr config)
{
	MikanComponent::setDefinition(config);

	auto transformComponentConfigPtr= std::static_pointer_cast<TransformComponentDefinition>(config);

	// Initially the model component isn't attached to anything
	assert(m_parentComponent.lock() == nullptr);
	m_worldTransform = transformComponentConfigPtr->getRelativeMat4();
	m_relativeTransform = GlmTransform(m_worldTransform);
}

bool TransformComponent::attachToComponent(TransformComponentPtr newParentComponent)
{
	TransformComponentPtr oldParentComponent= m_parentComponent.lock();

	if (!newParentComponent || newParentComponent == oldParentComponent)
		return false;

	// Detach from old parent
	if (oldParentComponent)
	{
		detachFromParent(eDetachReason::detachFromParent);
	}

	// Attach to new parent
	TransformComponentList& parentChildList= newParentComponent->m_childComponents;

	parentChildList.push_back(getSelfWeakPtr<TransformComponent>());
	m_parentComponent = newParentComponent;

	// Refresh world transform
	setRelativeTransform(m_relativeTransform);

	return true;
}

void TransformComponent::detachFromParent(eDetachReason reason)
{
	// Remove ourselves from our current parent's child list
	TransformComponentPtr parent= m_parentComponent.lock();
	if (parent != nullptr)
	{
		TransformComponentList& parentChildList= parent->m_childComponents;

		for (auto it = parentChildList.begin(); it != parentChildList.end(); ++it)
		{
			TransformComponentPtr transformComponent= it->lock(); 

			if (transformComponent.get() == this)
			{
				parentChildList.erase(it);
				break;
			}
		}
	}

	// Forget about our parent
	m_parentComponent = TransformComponentPtr();

	// World transform is now the same as relative transform
	m_worldTransform= m_relativeTransform.getMat4();

	// Only bother propagating our new transform to our children 
	// if we just doing a simple detach from our current parent
	if (reason == eDetachReason::detachFromParent)
	{
		propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);
	}
}

void TransformComponent::setRelativeTransform(const GlmTransform& newRelativeXform)
{
	// Set the new relative transform directly
	m_relativeTransform= newRelativeXform;
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bWasInitialized)
	{
		TransformComponentDefinitionPtr definitionPtr= getTransformComponentDefinition();
		if (definitionPtr)
			definitionPtr->setRelativeTransform(newRelativeXform);
	}
}

void TransformComponent::setRelativePosition(const glm::vec3& position)
{
	m_relativeTransform.setPosition(position);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bWasInitialized)
	{
		TransformComponentDefinitionPtr definitionPtr = getTransformComponentDefinition();
		if (definitionPtr)
			definitionPtr->setRelativePosition(glm_vec3_to_MikanVector3f(position));
	}
}

void TransformComponent::setRelativeRotation(const glm::quat& quat)
{
	m_relativeTransform.setRotation(quat);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bWasInitialized)
	{
		TransformComponentDefinitionPtr definitionPtr = getTransformComponentDefinition();
		if (definitionPtr)
			definitionPtr->setRelativeRotation(glm_quat_to_MikanQuatf(quat));
	}
}

void TransformComponent::setRelativeScale(const glm::vec3& scale)
{
	m_relativeTransform.setScale(scale);
	propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);

	if (m_bWasInitialized)
	{
		TransformComponentDefinitionPtr definitionPtr = getTransformComponentDefinition();
		if (definitionPtr)
			definitionPtr->setRelativeScale(glm_vec3_to_MikanVector3f(scale));
	}
}

void TransformComponent::setWorldTransform(const glm::mat4& newWorldXform)
{
	// Set the new world transform directly
	m_worldTransform= newWorldXform;

	// Subtract our new world transform from out parent to compute new relative transform
	glm::mat4 invParentXform= glm::mat4(1.f);
	TransformComponentPtr parent = m_parentComponent.lock();
	if (parent != nullptr)
	{
		invParentXform= glm::inverse(parent->getWorldTransform());
	}
	const glm::mat4 relativeXform= glm_composite_xform(m_worldTransform, invParentXform);
	m_relativeTransform= GlmTransform(relativeXform);

	propogateWorldTransformChange(eTransformChangeType::propogateWorldTransform);

	// If this component has an associated definition, update it too
	if (m_bWasInitialized)
	{
		TransformComponentDefinitionPtr definitionPtr= getTransformComponentDefinition();

		if (definitionPtr)
		{
			definitionPtr->setRelativeTransform(m_relativeTransform);
		}
	}
}

const glm::vec3 TransformComponent::getWorldLocation() const
{
	return glm_mat4_get_position(m_worldTransform);
}

void TransformComponent::propogateWorldTransformChange(eTransformChangeType reason)
{
	// Recompute our world transform, if requested
	if (reason == eTransformChangeType::recomputeWorldTransformAndPropogate)
	{
		// Concat our transform to our parent's world transform
		TransformComponentPtr parent = m_parentComponent.lock();
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
	for (TransformComponentWeakPtr childComponentWeakPtr : m_childComponents)
	{
		TransformComponentPtr childComponent= childComponentWeakPtr.lock();

		if (childComponent != nullptr)
		{
			// Ask all children to recompute their world transform
			childComponent->propogateWorldTransformChange(eTransformChangeType::recomputeWorldTransformAndPropogate);
		}
	}
}

// -- IPropertyInterface ----
void TransformComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(TransformComponentDefinition::k_relativeScalePropertyId);
	outPropertyNames.push_back(TransformComponentDefinition::k_relativeRotationPropertyId);
	outPropertyNames.push_back(TransformComponentDefinition::k_relativePositionPropertyId);
}

bool TransformComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == TransformComponentDefinition::k_relativeScalePropertyId)
	{
		outDescriptor = {TransformComponentDefinition::k_relativeScalePropertyId, ePropertyDataType::datatype_float3, ePropertySemantic::scale};
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativeRotationPropertyId)
	{
		outDescriptor = {TransformComponentDefinition::k_relativeRotationPropertyId, ePropertyDataType::datatype_float3, ePropertySemantic::rotation};
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativePositionPropertyId)
	{
		outDescriptor = {TransformComponentDefinition::k_relativePositionPropertyId, ePropertyDataType::datatype_float3, ePropertySemantic::position};
		return true;
	}

	return false;
}

bool TransformComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == TransformComponentDefinition::k_relativeScalePropertyId)
	{
		const glm::vec3& scale = getRelativeScale();
		outValue = Rml::Vector3f(scale.x, scale.y, scale.z);
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativeRotationPropertyId)
	{
		const glm::quat& model_orientation = getRelativeRotation();

		float angles[3]{};
		glm_quat_to_euler_angles(model_orientation, angles[0], angles[1], angles[2]);
		angles[0] *= k_radians_to_degrees;
		angles[1] *= k_radians_to_degrees;
		angles[2] *= k_radians_to_degrees;

		outValue = Rml::Vector3f(angles[0], angles[1], angles[2]);
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativePositionPropertyId)
	{
		const glm::vec3& pos = getRelativePosition();
		outValue = Rml::Vector3f(pos.x, pos.y, pos.z);
		return true;
	}

	return false;
}

bool TransformComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == TransformComponentDefinition::k_relativeScalePropertyId)
	{
		Rml::Vector3 scale = inValue.Get<Rml::Vector3f>();

		setRelativeScale(glm::vec3(scale.x, scale.y, scale.z));
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativeRotationPropertyId)
	{
		Rml::Vector3 angles = inValue.Get<Rml::Vector3f>();

		glm::quat quat;
		glm_euler_angles_to_quat(
			angles.x * k_degrees_to_radians, 
			angles.y * k_degrees_to_radians, 
			angles.z * k_degrees_to_radians, 
			quat);
		setRelativeRotation(quat);
		return true;
	}
	else if (propertyName == TransformComponentDefinition::k_relativePositionPropertyId)
	{
		Rml::Vector3 pos = inValue.Get<Rml::Vector3f>();

		setRelativePosition(glm::vec3(pos.x, pos.y, pos.z));
		return true;
	}

	return false;
}