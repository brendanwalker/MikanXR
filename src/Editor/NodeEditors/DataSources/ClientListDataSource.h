#pragma once

#include "NodeEditorUI.h"
#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include <vector>

class ClientListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ClientListDataSource(ClientTextureSourceSystemPtr ClientTextureSourceSystem);

	int getEntryIndex(ClientTextureSourceComponentPtr TextureSourceComponent) const;
	ClientTextureSourceComponentPtr getEntryAtIndex(int index) const;

	virtual int getEntryCount() override;
	virtual const std::string& getEntryDisplayString(int index) override;

private:
	std::vector<ClientTextureSourceComponentPtr> comboEntrieValues;
	std::vector<std::string> comboEntrieNames;
};