#include "ClientListDataSource.h"

#include "GlFrameCompositor.h"
#include "MainWindow.h"

ClientListDataSource::ClientListDataSource()
{
	GlFrameCompositor* compositor = MainWindow::getInstance()->getFrameCompositor();
	if (compositor != nullptr)
	{
		auto& clientSources= compositor->getClientSources();

		for (auto it = clientSources.getMap().begin(); it != clientSources.getMap().end(); it++)
		{
			GlFrameCompositor::ClientSource* clientSource= it->second;

			comboEntries.push_back({clientSource->clientSourceIndex, clientSource->clientId});
		}
	}

	if (comboEntries.size() == 0)
	{
		comboEntries.push_back({0, "Client 0"});
	}
}