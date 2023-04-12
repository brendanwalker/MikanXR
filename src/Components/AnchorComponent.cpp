#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"

AnchorComponent::AnchorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, AnchorId(INVALID_MIKAN_ID)
	, AnchorXform(glm::mat4(1.f))
	, AnchorName("")
{}

void AnchorComponent::init()
{
	MikanComponent::init();

	m_sceneComponent = getOwnerObject()->getComponentOfType<SceneComponent>();
}

void AnchorComponent::setSpatialAnchor(const MikanSpatialAnchorInfo& anchor)
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

void AnchorComponent::setAnchorXform(const glm::mat4& xform)
{
	AnchorXform= xform;
	notifyAnchorXformChanged();
	updateSceneComponentTransform();
}

void AnchorComponent::setAnchorName(const std::string& newAnchorName)
{
	AnchorName= newAnchorName;
	notifyAnchorNameChanged();
}

void AnchorComponent::updateSceneComponentTransform()
{
	SceneComponentPtr sceneComponent = m_sceneComponent.lock();
	if (sceneComponent)
	{
		const glm::mat4 worldXform = getAnchorXform();

		sceneComponent->setWorldTransform(worldXform);
	}
}