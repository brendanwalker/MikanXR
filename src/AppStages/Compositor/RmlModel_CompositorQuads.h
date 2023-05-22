#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorQuad
{
	Rml::String stencil_name;
	int stencil_id;
	int parent_anchor_id;
	Rml::Vector3f position;
	Rml::Vector3f angles;
	Rml::Vector2f size;
	bool double_sided;
	bool disabled;
};

class RmlModel_CompositorQuads : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		StencilObjectSystemPtr stencilSystemPtr);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnAddQuadStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteQuadStencilEvent;

private:
	RmlModel_CompositorQuad* getQuadRmlModel(const int stencil_id);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void rebuildAnchorList();
	void rebuildUIQuadsFromStencilSystem();
	void copyStencilSystemToUIQuad(int stencil_id);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorQuad> m_stencilQuads;

	static bool s_bHasRegisteredTypes;
};
