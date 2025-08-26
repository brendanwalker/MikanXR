#pragma once

#include "ComponentFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"

class RmlModel_CompositorLayers : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		CompositorComponentPtr compositor);
	virtual void dispose() override;

	const std::filesystem::path getCompositorGraphPath() const;
	void setCompositorGraphPath(const std::filesystem::path& path);
	SinglecastDelegate<void()> OnGraphEditEvent;
	SinglecastDelegate<void()> OnGraphFileSelectEvent;

	void onCompositorConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

private:
	CompositorComponentPtr m_compositor;
	CompositorDefinitionPtr m_compositorDefinition;

	Rml::String m_compositorGraphPath;
};
