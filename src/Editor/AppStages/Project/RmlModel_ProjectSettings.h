#pragma once

#include "ComponentFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

#include <vector>
#include <string>

class RmlModel_ProjectSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		ProjectConfigPtr profileConfig,
		StencilObjectSystemPtr stencilSystem);
	virtual void dispose() override;

private:
	ProjectConfigWeakPtr m_project;
	StencilObjectSystemWeakPtr m_stencilSystem;
	std::vector<std::string> m_languageIdList;
	std::string m_selectedLangugeId;
};
