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

	bool init(
		Rml::Context* rmlContext, 
		TextureSourceSystemPtr textureSourceSystem,
		VideoSourceSystemPtr videoSourceSystem);
	virtual void dispose() override;

private:
	VideoSourceSystemPtr getVideoSourceSystem();
	VideoSourceComponentPtr getSelectedVideoSource();
	USBVideoSourceComponentPtr getSelectedUSBVideoSource();
	NetworkVideoSourceComponentPtr getSelectedNetworkVideoSource();

	void addNewUSBVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewNetworkVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeVideoSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectVideoSourceEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void setSelectedVideoSourceId(MikanVideoSourceID videoSourceId);
	void videoSourceIdListChanged(bool bOwnerChanged);

	TextureSourceSystemPtr getTextureSourceSystem();
	TextureSourceComponentPtr getSelectedTextureSource();
	ClientTextureSourceComponentPtr getSelectedClientTextureSource();
	SpoutTextureSourceComponentPtr getSelectedSpoutTextureSource();

	void addNewClientTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewSpoutTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTextureSource(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTextureSourceEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void setSelectedTextureSourceId(MikanTextureSourceID textureSourceId);
	void textureSourceIdListChanged(bool bOwnerChanged);

	TextureSourceSystemWeakPtr m_textureSourceSystem;
	VideoSourceSystemWeakPtr m_videoSourceSystem;

	RmlDataBinding_ComponentIdListPtr m_videoSourceIdList;
	RmlModel_USBVideoSourceComponentPtr m_selectedUSBVideoSourceModel;
	RmlModel_NetworkVideoSourceComponentPtr m_selectedNetworkVideoSourceModel;

	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;
	RmlModel_ClientTextureSourceComponentPtr m_selectedClientVideoSourceModel;
	RmlModel_SpoutTextureSourceComponentPtr m_selectedSpoutVideoSourceModel;

	int m_selectedVideoSourceId = -1; // MikanVideoSourceID
	int m_selectedTextureSourceId = -1; // MikanTextureSourceID
};
