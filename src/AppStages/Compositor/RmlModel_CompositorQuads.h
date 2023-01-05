#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorQuad
{
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
	bool init(Rml::Context* rmlContext, const class ProfileConfig* profile);
	virtual void dispose() override;

	void rebuildAnchorList(const ProfileConfig* profile);
	void rebuildUIQuadsFromProfile(const ProfileConfig* profile);
	void copyUIQuadToProfile(int stencil_id, ProfileConfig* profile) const;

	SinglecastDelegate<void()> OnAddQuadStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteQuadStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnModifyQuadStencilEvent;

private:
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorQuad> m_stencilQuads;

	static bool s_bHasRegisteredTypes;
};
