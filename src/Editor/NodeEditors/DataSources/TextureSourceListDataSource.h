#pragma once

#include "MkGuiDrawUtils.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemFwd.h"
#include <vector>

class TextureSourceListDataSource : public MkGui::ComboBoxDataSource
{
public:
	TextureSourceListDataSource(ProjectManagerPtr projectManager);

	int getEntryIndex(TextureSourceComponentPtr textureSourceComponent) const;
	TextureSourceComponentPtr getEntryAtIndex(int index) const;

	virtual int getEntryCount() const override;
	virtual const std::string& getEntryDisplayString(int index) const override;

private:
	std::vector<TextureSourceComponentPtr> comboEntrieValues;
	std::vector<std::string> comboEntrieNames;
};