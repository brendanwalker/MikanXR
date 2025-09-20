#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "SinglecastDelegate.h"

class RmlModel_ProjectSources : public RmlModel
{
public:
	RmlModel_ProjectSources();

	bool init(
		Rml::Context* rmlContext, 
		ProjectConfigPtr projectConfig,
		VideoSourceSystemPtr videoSourceSystem);
	virtual void dispose() override;

private:
	VideoSourceSystemPtr getVideoSourceSystem();
	VideoSourceComponentPtr getSelectedVideoSource();
	ClientVideoSourceComponentPtr getSelectedClientVideoSource();
	USBVideoSourceComponentPtr getSelectedUSBVideoSource();
	NetworkVideoSourceComponentPtr getSelectedNetworkVideoSource();
	SpoutVideoSourceComponentPtr getSelectedSpoutVideoSource();

	void addNewClientVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewUSBVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewNetworkVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewSpoutVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectVideoSourceEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedVideoSourceId(MikanVideoSourceID videoSourceId);

	void videoSourceIdListChanged(bool bOwnerChanged);

	ProjectConfigWeakPtr m_projectConfig;
	VideoSourceSystemWeakPtr m_videoSourceSystem;

	RmlDataBinding_ComponentIdListPtr m_videoSourceIdList;
	RmlModel_PropertyInterfacePtr m_selectedClientVideoSourceModel;
	RmlModel_USBVideoSourceComponentPtr m_selectedUSBVideoSourceModel;
	RmlModel_PropertyInterfacePtr m_selectedNetworkVideoSourceModel;
	RmlModel_PropertyInterfacePtr m_selectedSpoutVideoSourceModel;

	int m_selectedVideoSourceId = -1; // MikanVideoSourceID
};
