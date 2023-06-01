#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

struct RmlModel_CompositorComponent
{
	Rml::String name;
	int depth;
};

class RmlModel_CompositorOutliner : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		StencilObjectSystemPtr stencilSystemPtr);
	virtual void dispose() override;

private:
	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void rebuildComponentList();
	void addSceneComponent(SceneComponentPtr sceneComponentPtr, int depth);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<RmlModel_CompositorComponent> m_componentOutliner;

	static bool s_bHasRegisteredTypes;
};
