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
	bool init(class ProjectRmlModelContext* context);

private:
	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;
	ProjectConfigWeakPtr m_project;
	AnchorObjectSystemWeakPtr m_anchorSystem;
	StencilObjectSystemWeakPtr m_stencilSystem;
	std::vector<std::string> m_languageIdList;
	std::string m_selectedLangugeId;
};
