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

	SinglecastDelegate<void()> OnAddBoxStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteBoxStencilEvent;

private:
	RmlModel_CompositorBox* getBoxRmlModel(const int stencil_id);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void rebuildAnchorList();
	void rebuildUIBoxesFromStencilSystem();
	void copyStencilSystemToUIBox(int stencil_id);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;

	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorBox> m_stencilBoxes;

	static bool s_bHasRegisteredTypes;
};
