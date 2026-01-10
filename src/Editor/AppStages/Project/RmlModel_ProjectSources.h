#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "SinglecastDelegate.h"

class RmlModel_ProjectSources : public RmlModel
{
public:
	RmlModel_ProjectSources();

	bool init(class ProjectRmlModelContext* context);
	virtual void dispose() override;

private:
	VideoSourceComponentPtr getSelectedVideoSource();
	USBVideoSourceComponentPtr getSelectedUSBVideoSource();
	NetworkVideoSourceComponentPtr getSelectedNetworkVideoSource();

	void addNewUSBVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewNetworkVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectVideoSourceEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void setSelectedVideoSourceId(MikanVideoSourceID videoSourceId);
	void videoSourceIdListChanged(bool bOwnerChanged);

	TextureSourceComponentPtr getSelectedTextureSource();
	ClientTextureSourceComponentPtr getSelectedClientTextureSource();
	SpoutTextureSourceComponentPtr getSelectedSpoutTextureSource();

	void addNewClientTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewSpoutTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTextureSourceEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void setSelectedTextureSourceId(MikanTextureSourceID textureSourceId);
	void textureSourceIdListChanged(bool bOwnerChanged);

	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;
	ProjectManagerWeakPtr m_projectManager;

	RmlDataBinding_ComponentIdListPtr m_videoSourceIdList;
	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;

	int m_selectedVideoSourceId = -1; // MikanVideoSourceID
	int m_selectedTextureSourceId = -1; // MikanTextureSourceID
};
