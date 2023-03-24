#pragma once

//-- includes -----
#include "Shared/ModalDialog.h"
#include "SinglecastDelegate.h"

#include <string>
#include <vector>
#include <functional>

class AppStage;

namespace Rml
{
	class ElementDocument;
}

/// The ID of a spatial anchor
typedef int32_t MikanSpatialFastenerID;

//-- definitions -----

class ModalDialog_Snap : public ModalDialog
{
public:
	ModalDialog_Snap(AppStage* ownerAppStage);
	virtual ~ModalDialog_Snap();

	using SnapCallback = std::function<void(MikanSpatialFastenerID sourceId, MikanSpatialFastenerID targetId)>;
	using CancelCallback = std::function<void()>;
	static bool selectSnapTarget(
		MikanSpatialFastenerID sourceId,
		SnapCallback acceptCallback={},
		CancelCallback rejectCallback={});

protected:
	class RmlModel_Snap* m_snapModel = nullptr;
	Rml::ElementDocument* m_snapView = nullptr;

	SnapCallback m_requestSnapCallback;
	CancelCallback m_cancelCallback;

	bool init(
		class ProfileConfig* profile,
		MikanSpatialFastenerID sourceId,
		SnapCallback acceptCallback,
		CancelCallback rejectCallback);
	void onRequestFastenerSnap(int sourceFastenerId, int targetFastenerId);
	void onCancelSnap();
};
