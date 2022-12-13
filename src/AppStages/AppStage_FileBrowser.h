#pragma once

//-- includes -----
#include "AppStage.h"
#include "SinglecastDelegate.h"

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

	void setInitialDirectory(const std::string& initial_directory);
	SinglecastDelegate<void(const std::string &filepath)> OnAcceptFilePath;
	SinglecastDelegate<void()> OnRejectFilePath;

	virtual void enter() override;
	
	virtual void onRmlClickEvent(const std::string& value) override;

	static const char* APP_STAGE_NAME;

protected:
	struct FileBrowserData* m_data= nullptr;
};
