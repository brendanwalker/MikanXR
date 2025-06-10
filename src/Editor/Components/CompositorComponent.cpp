#include "CompositorComponent.h"
#include "App.h"
#include "MainWindow.h"
#include "ProjectConfig.h"
#include "TransformComponent.h"
#include "SelectionComponent.h"
#include "MikanObject.h"
#include "MathTypeConversion.h"
#include "StringUtils.h"

// -- CompositorConfig -----
CompositorDefinition::CompositorDefinition()
	: MikanComponentDefinition()
{
	m_compositorId = INVALID_MIKAN_ID;
}

CompositorDefinition::CompositorDefinition(
	MikanCompositorID compositorId,
	const std::string& compositorName)
	: MikanComponentDefinition(compositorName)
	, m_compositorId(compositorId)
{}

configuru::Config CompositorDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_compositorId;

	return pt;
}

void CompositorDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_compositorId = pt.get<int>("id");
}

// -- CompositorComponent -----
CompositorComponent::CompositorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
	m_bWantsCustomRender = true;
}

void CompositorComponent::init()
{
	MikanComponent::init();

}