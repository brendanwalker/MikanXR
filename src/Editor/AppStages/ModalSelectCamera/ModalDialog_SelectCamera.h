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
	virtual ~ModalDialog_SelectCamera() = default;

	using SelectCallback = std::function<void(MikanCameraID)>;
	using CancelCallback = std::function<void()>;
	static bool selectCamera(
		AppStage* appStage,
		SelectCallback selectCallback={},
		CancelCallback cancelCallback={});

	virtual void onGui() override;

protected:
	SelectCallback m_selectCallback;
	CancelCallback m_cancelCallback;

	std::vector<MikanCameraID> m_cameraIds;
	std::vector<std::string> m_cameraNames;
	int m_selectedIndex = 0;

	bool init(SelectCallback selectCallback, CancelCallback cancelCallback);
	void onSelectCamera();
	void onCancel();
};
