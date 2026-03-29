#pragma once

//-- includes -----
#include "RmlFwd.h"
#include "MikanTypeFwd.h"
#include "SceneFwd.h"
#include "Shared/ModalDialog.h"
#include "SinglecastDelegate.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

//-- definitions -----

class ModalDialog_SceneAddCompositor : public ModalDialog
{
public:
	ModalDialog_SceneAddCompositor(AppStage* ownerAppStage);
	virtual ~ModalDialog_SceneAddCompositor();

	using SelectCallback = std::function<void(MikanCompositorID)>;
	using CancelCallback = std::function<void()>;
	static bool selectNewCompositor(
		AppStage* appStage,
		SceneComponentPtr ownerScene,
		SelectCallback selectCallback={},
		CancelCallback cancelCallback={});

protected:
	class RmlModel_SceneAddCompositor* m_selectCompositorModel = nullptr;
	Rml::ElementDocument* m_selectCompositorView = nullptr;

	SelectCallback m_selectCallback;
	CancelCallback m_cancelCallback;

	bool init(
		SceneComponentPtr ownerScene,
		SelectCallback selectCallback,
		CancelCallback cancelCallback);
	void onSelectCompositor(MikanCompositorID compositorId);
	void onCancel();
};
