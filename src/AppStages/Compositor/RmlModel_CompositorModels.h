#pragma once

#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

struct RmlModel_CompositorModel
{
	Rml::String stencil_name;
	int stencil_id;
	int parent_anchor_id;
	Rml::String model_path;
	Rml::Vector3f model_position;
	Rml::Vector3f model_angles;
	Rml::Vector3f model_scale;
	bool disabled;
};

class RmlModel_CompositorModels : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		AnchorObjectSystemPtr anchorSystemPtr,
		StencilObjectSystemPtr objectSystemPtr);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnAddModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteModelStencilEvent;

private:
	RmlModel_CompositorModel* getModelRmlModel(const int stencil_id);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void rebuildAnchorList();
	void rebuildStencilUIModelsFromProfile();

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;

	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorModel> m_stencilModels;

	static bool s_bHasRegisteredTypes;
};
