#include "MikanAnchorComponent.h"
#include "MikanSceneComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"

MikanAnchorComponent::MikanAnchorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, AnchorId(INVALID_MIKAN_ID)
	, AnchorXform(glm::mat4(1.f))
	, AnchorName("")
{}

void MikanAnchorComponent::init()
{
	MikanComponent::init();

	m_sceneComponent = getOwnerObject()->getComponentOfType<MikanSceneComponent>();
}

void MikanAnchorComponent::setSpatialAnchor(const MikanSpatialAnchorInfo& anchor)
{
	AnchorId= anchor.anchor_id;
	AnchorXform= MikanMatrix4f_to_glm_mat4(anchor.anchor_xform);
	AnchorName= anchor.anchor_name;

	if (ComponentPropertyEvents::OnComponentChanged)
	{
		ComponentPropertyEvents::OnComponentChanged(*this);
	}

	updateSceneComponentTransform();
}

void MikanAnchorComponent::setAnchorXform(const glm::mat4& xform)
{
	AnchorXform= xform;
	notifyAnchorXformChanged();
	updateSceneComponentTransform();
}

void MikanAnchorComponent::setAnchorName(const std::string& newAnchorName)
{
	AnchorName= newAnchorName;
	notifyAnchorNameChanged();
}

void MikanAnchorComponent::updateSceneComponentTransform()
{
	MikanSceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getAnchorXform();

		sceneComponent->setWorldTransform(worldXform);
	}
}