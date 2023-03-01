#pragma once

//-- includes -----
#include "Shared/ModalDialog.h"
#include "SinglecastDelegate.h"

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class AppStage;

namespace Rml
{
	class ElementDocument;
}

//-- definitions -----

class ModalDialog_FileBrowser : public ModalDialog
{
public:
	ModalDialog_FileBrowser(AppStage* ownerAppStage);
	virtual ~ModalDialog_FileBrowser();

	using AcceptFilePathCallback = std::function<void(const std::filesystem::path& filepath)>;
	using RejectFilePathCallback = std::function<void()>;
	using ErrorCallback = std::function<void(const std::string& errorMesg)>;
	static bool browseFile(
		const std::string& title,
		const std::filesystem::path& initialDirectory,
		const std::vector<std::string>& typeFilters,
		AcceptFilePathCallback acceptCallback={},
		RejectFilePathCallback rejectCallback={});

protected:
	class RmlModel_FileBrowser* m_fileBrowserModel = nullptr;
	Rml::ElementDocument* m_fileBrowserView = nullptr;

	AcceptFilePathCallback m_acceptCallback;
	RejectFilePathCallback m_rejectCallback;

	bool init(
		const std::string& title,
		const std::filesystem::path& initialDirectory,
		const std::vector<std::string>& typeFilters,
		AcceptFilePathCallback acceptCallback,
		RejectFilePathCallback rejectCallback);
	void onAcceptFilePath(const std::filesystem::path& filepath);
	void onRejectFilePath();
};
