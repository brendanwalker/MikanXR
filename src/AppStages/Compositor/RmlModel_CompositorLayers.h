#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"
#include "CompositorFwd.h"

class RmlModel_CompositorLayers : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		class GlFrameCompositor* compositor);
	virtual void dispose() override;

	const std::filesystem::path getCompositorGraphPath() const;
	void setCompositorGraphPath(const std::filesystem::path& path);
	SinglecastDelegate<void()> OnGraphEditEvent;
	SinglecastDelegate<void()> OnGraphFileSelectEvent;

	SinglecastDelegate<void()> OnConfigAddEvent;
	SinglecastDelegate<void()> OnConfigDeleteEvent;
	SinglecastDelegate<void(const Rml::String& newConfigName)> OnConfigNameChangeEvent;
	SinglecastDelegate<void(const Rml::String& configName)> OnConfigSelectEvent;

	void onCompositorConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void onCurrentPresetChanged();
	void onPresetConfigMarkedDirty(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);
	void rebuild(const class GlFrameCompositor* compositor);

private:
	class GlFrameCompositor* m_compositor= nullptr;
	GlFrameCompositorConfigPtr m_compositorConfig;
	CompositorPresetPtr m_presetConfig;

	Rml::String m_currentConfigurationName;
	bool m_bIsBuiltInConfiguration;
	Rml::String m_compositorGraphPath;
	Rml::Vector<Rml::String> m_configurationNames;
};
