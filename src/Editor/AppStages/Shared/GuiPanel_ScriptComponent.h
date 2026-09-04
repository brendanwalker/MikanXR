#pragma once

#include "Shared/GuiPanel_MikanComponent.h"
#include "ScriptComponent.h"

class GuiPanel_ScriptComponent : public GuiPanel_MikanComponent
{
public:
	GuiPanel_ScriptComponent(AppStage* ownerAppStage)
		: GuiPanel_MikanComponent(ownerAppStage)
	{
	}

	virtual bool init() override;
	virtual void onConstruct() override;
	virtual void onGui() override;

protected:
	ScriptComponentPtr getScriptComponent() const;
	void drawScriptVariable(ScriptComponentPtr component, const std::string& variableName,
							class TransactionHistory* transactionHistory);
};

using GuiPanel_ScriptComponentPtr= std::shared_ptr<GuiPanel_ScriptComponent>;
