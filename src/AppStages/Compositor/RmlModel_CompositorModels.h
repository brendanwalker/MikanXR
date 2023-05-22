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
	Rml::Vector<int> child_fastener_ids;
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
		StencilObjectSystemPtr objectSystemPtr,
		FastenerObjectSystemPtr fastenerSystemPtr);
	virtual void dispose() override;

	SinglecastDelegate<void()> OnAddModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnDeleteModelStencilEvent;
	SinglecastDelegate<void(int stencilID)> OnSelectModelStencilPathEvent;
	SinglecastDelegate<void(int stencilID)> OnAddFastenerEvent;
	SinglecastDelegate<void(int fastenerID)> OnSnapFastenerEvent;
	SinglecastDelegate<void(int fastenerID)> OnEditFastenerEvent;
	SinglecastDelegate<void(int stencilID, int fastenerID)> OnDeleteFastenerEvent;

private:
	RmlModel_CompositorModel* getModelRmlModel(const int stencil_id);

	void anchorSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void stencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void fastenerSystemConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void rebuildAnchorList();
	void rebuildStencilUIModelsFromProfile();
	void copyStencilSystemToUIModel(int stencil_id);

	AnchorObjectSystemPtr m_anchorSystemPtr;
	StencilObjectSystemPtr m_stencilSystemPtr;
	FastenerObjectSystemPtr m_fastenerSystemPtr;

	Rml::Vector<int> m_spatialAnchors;
	Rml::Vector<RmlModel_CompositorModel> m_stencilModels;

	static bool s_bHasRegisteredTypes;
};
