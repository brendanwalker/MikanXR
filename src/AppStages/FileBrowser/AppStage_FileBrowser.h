#pragma once

//-- includes -----
#include "AppStage.h"
#include "SinglecastDelegate.h"

#include <string>
#include <vector>
#include <functional>

namespace Rml
{
	class ElementDocument;
}

//-- definitions -----

class AppStage_FileBrowser : public AppStage
{
public:
	AppStage_FileBrowser(class App* app);
	virtual ~AppStage_FileBrowser();

	virtual void enter() override;
	virtual void exit() override;

	using AcceptFilePathCallback = std::function<void(const std::string& filepath)>;
	using RejectFilePathCallback = std::function<void()>;
	static bool browseFile(
		const std::string& title,
		const std::string& initialDirectory,
		const std::vector<std::string>& typeFilters,
		AcceptFilePathCallback acceptCallback={},
		RejectFilePathCallback rejectCallback={});

	static const char* APP_STAGE_NAME;

protected:
	class RmlModel_FileBrowser* m_filebrowserModel = nullptr;
	Rml::ElementDocument* m_filebrowserView = nullptr;

	AcceptFilePathCallback m_acceptCallback;
	RejectFilePathCallback m_rejectCallback;

	void onAcceptFilePath(const Rml::String& filepath);
	void onRejectFilePath();
};
