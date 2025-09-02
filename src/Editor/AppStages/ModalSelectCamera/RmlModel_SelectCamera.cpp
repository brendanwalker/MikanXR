#include "RmlModel_SelectCamera.h"
#include "CameraObjectSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

bool RmlModel_SelectCamera::init(
	Rml::Context* rmlContext,
	CameraObjectSystemPtr cameraSystem)
{
	// Create Datamodel
	Rml::DataModelConstructor constructor = RmlModel::init(rmlContext, "select_camera");
	if (!constructor)
		return false;

	constructor.Bind("camera_id_list", &m_cameraIdList);
	constructor.Bind("selected_camera_id", &m_selectedCameraIndex);
	constructor.BindEventCallback(
		"camera_id_changed", 
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
			m_selectedCameraIndex = ev.GetParameter<int>("value", -1);
		});
	constructor.BindEventCallback(
		"ok_action",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnOk) OnOk(m_selectedCameraIndex);
		});
	constructor.BindEventCallback(
		"cancel_action",
		[this](Rml::DataModelHandle model, Rml::Event& /*ev*/, const Rml::VariantList& arguments) {
			if (OnCancel) OnCancel();
		});

	m_cameraIdList = cameraSystem->getAllCameraIds();
	m_modelHandle.DirtyVariable("camera_id_list");

	return true;
}

void RmlModel_SelectCamera::dispose()
{
	OnOk.Clear();
	OnCancel.Clear();
	RmlModel::dispose();
}