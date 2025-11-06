#pragma once

#include "NodeEditorUI.h"
#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include <vector>

class ClientListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ClientListDataSource(ClientVideoSourceSystemPtr clientVideoSourceSystem);

	int getEntryIndex(ClientVideoSourceComponentPtr videoSourceComponent) const;
	ClientVideoSourceComponentPtr getEntryAtIndex(int index) const;

	virtual int getEntryCount() override;
	virtual const std::string& getEntryDisplayString(int index) override;

private:
	std::vector<ClientVideoSourceComponentPtr> comboEntrieValues;
	std::vector<std::string> comboEntrieNames;
};