#pragma once

//-- includes -----
#include "MikanTypeFwd.h"
#include "SceneFwd.h"
#include "Shared/ModalDialog.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_SceneAddCompositor : public ModalDialog
{
public:
	ModalDialog_SceneAddCompositor(AppStage* ownerAppStage);
	virtual ~ModalDialog_SceneAddCompositor() = default;

	using SelectCallback = std::function<void(MikanCompositorID)>;
	using CancelCallback = std::function<void()>;
	static bool selectNewCompositor(
		AppStage* appStage,
		SceneComponentPtr ownerScene,
		SelectCallback selectCallback={},
		CancelCallback cancelCallback={});

	virtual void onGui() override;

protected:
	SelectCallback m_selectCallback;
	CancelCallback m_cancelCallback;

	std::vector<MikanCompositorID> m_compositorIds;
	std::vector<std::string> m_compositorNames;
	int m_selectedIndex = 0;

	bool init(
		SceneComponentPtr ownerScene,
		SelectCallback selectCallback,
		CancelCallback cancelCallback);
	void onSelectCompositor();
	void onCancel();
};
