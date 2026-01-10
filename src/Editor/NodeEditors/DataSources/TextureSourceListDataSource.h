#pragma once

#include "NodeEditorUI.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include <vector>

class TextureSourceListDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	TextureSourceListDataSource(ProjectManagerPtr projectManager);

	int getEntryIndex(TextureSourceComponentPtr textureSourceComponent) const;
	TextureSourceComponentPtr getEntryAtIndex(int index) const;

	virtual int getEntryCount() override;
	virtual const std::string& getEntryDisplayString(int index) override;

private:
	std::vector<TextureSourceComponentPtr> comboEntrieValues;
	std::vector<std::string> comboEntrieNames;
};