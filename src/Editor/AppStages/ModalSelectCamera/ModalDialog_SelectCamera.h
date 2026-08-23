#pragma once

//-- includes -----
#include "MikanTypeFwd.h"
#include "Shared/ModalDialog.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_SelectCamera : public ModalDialog
{
public:
	ModalDialog_SelectCamera(AppStage* ownerAppStage);
	virtual ~ModalDialog_SelectCamera()= default;

	using SelectCallback= std::function<void(MikanCameraID)>;
	using CancelCallback= std::function<void()>;
	using FilterCallback= std::function<bool(CameraComponentPtr)>;
	static bool selectCamera(AppStage* appStage, SelectCallback selectCallback= {}, CancelCallback cancelCallback= {},
							 FilterCallback filterCallback= {});

	virtual void onGui() override;

protected:
	SelectCallback m_selectCallback;
	CancelCallback m_cancelCallback;
	FilterCallback m_filterCallback;

	std::vector<MikanCameraID> m_cameraIds;
	std::vector<std::string> m_cameraNames;
	int m_selectedIndex= 0;

	bool init(SelectCallback selectCallback, CancelCallback cancelCallback, FilterCallback filterCallback);
	void onSelectCamera();
	void onCancel();
};
