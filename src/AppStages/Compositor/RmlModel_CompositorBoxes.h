#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorBox
{
	Rml::String stencil_name;
	int stencil_id;
	int parent_anchor_id;
	Rml::Vector3f box_center;
	Rml::Vector3f angles;
	Rml::Vector3f size;
	bool disabled;
};

class RmlModel_CompositorBoxes : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		AnchorObjectSystemPtr anchorSystemPtr,
		StencilObjectSystemPtr objectSystemPtr);
	virtual void dispose() override;

	void rebuildAnchorList();
	void rebuildUIBoxesFromStencilSystem();
	void copyUIBoxToStencilSystem(int stencil_id) const;

	SinglecastDelegate<void()> OnAddBoxStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteBoxStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnModifyBoxStencilEvent;
	SinglecastDelegate<void(int stencilID, int anchorId)> OnModifyBoxStencilParentAnchorEvent;

private:
	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorBox> m_stencilBoxes;

	static bool s_bHasRegisteredTypes;
};
