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
	SinglecastDelegate<void(int stencilID, int anchorId)> OnModifyQuadStencilParentAnchorEvent;

private:
	void rebuildAnchorList();
	void rebuildUIQuadsFromProfile();
	void copyUIQuadToStencilSystem(int stencil_id) const;
	void copyStencilSystemToUIQuad(int stencil_id);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorQuad> m_stencilQuads;

	static bool s_bHasRegisteredTypes;
};
