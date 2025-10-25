#include "RmlModel_SceneAddCompositor.h"
#include "CompositorObjectSystem.h"
#include "SceneComponent.h"
#include "StageComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_SceneAddCompositor::init(
	Rml::Context* rmlContext,
	CompositorObjectSystemPtr compositorSystem,
	SceneComponentPtr ownerScene)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "select_scene_compositor");
	if (!constructor)
		return false;

	constructor.Bind("compositor_id_list", &m_compositorIdList);
	constructor.Bind("selected_compositor_id", &m_selectedCompositorId);
	constructor.BindEventCallback(
		"compositor_id_changed", 
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_selectedCompositorId = ev.GetParameter<int>("value", -1);
		});
	constructor.BindEventCallback(
		"ok_action",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnOk) OnOk(m_selectedCompositorId);
		});
	constructor.BindEventCallback(
		"cancel_action",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancel) OnCancel();
		});

	// We can select from any compositors used by the parent stage not already added to the scene
	m_compositorIdList.clear();
	StageComponentPtr stageComponent= ownerScene->getParentStage();
	if (!stageComponent)
	{
		const MikanStageID stageId= stageComponent->getStageId();
		const std::vector<MikanCompositorID>& existingIDs= ownerScene->getOutputCompositorIDs();
		const std::vector<MikanCompositorID> availableIDs = compositorSystem->getCompositorIdListForStage(stageId);

		for (const MikanCompositorID& id : availableIDs)
		{
			auto it = std::find(existingIDs.begin(), existingIDs.end(), id);
			if (it != existingIDs.end())
			{
				m_compositorIdList.push_back(id);
			}
		}
	}

	m_modelHandle.DirtyVariable("compositor_id_list");

	return true;
}

void RmlModel_SceneAddCompositor::dispose()
{
	OnOk.Clear();
	OnCancel.Clear();
	RmlModel::dispose();
}