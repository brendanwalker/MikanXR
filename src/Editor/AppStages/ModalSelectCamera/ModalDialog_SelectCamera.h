#pragma once

//-- includes -----
#include "RmlFwd.h"
#include "MikanTypeFwd.h"
#include "Shared/ModalDialog.h"
#include "SinglecastDelegate.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_SelectCamera : public ModalDialog
{
public:
	ModalDialog_SelectCamera(AppStage* ownerAppStage);
	virtual ~ModalDialog_SelectCamera();

	using SelectCallback = std::function<void(MikanCameraID)>;
	using CancelCallback = std::function<void()>;
	static bool selectCamera(
		AppStage* appStage,
		SelectCallback selectCallback={},
		CancelCallback cancelCallback={});

protected:
	class RmlModel_SelectCamera* m_selectCameraModel = nullptr;
	Rml::ElementDocument* m_selectCameraView = nullptr;

	SelectCallback m_selectCallback;
	CancelCallback m_cancelCallback;

	bool init(
		SelectCallback selectCallback,
		CancelCallback cancelCallback);
	void onSelectCamera(MikanCameraID cameraId);
	void onCancel();
};
