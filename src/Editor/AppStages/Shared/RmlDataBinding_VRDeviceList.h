#pragma once

#include "Shared\RmlDataBinding.h"

class RmlDataBinding_VRDeviceList : public RmlDataBinding
{
public:
	RmlDataBinding_VRDeviceList()= default;
	virtual ~RmlDataBinding_VRDeviceList();

	virtual bool init(Rml::DataModelConstructor constructor) override;
	virtual void dispose() override;

	const Rml::Vector<Rml::String>& getVideoDeviceList() const { return m_vrDeviceList; }

protected:
	void rebuildVRDeviceList();

private:
	Rml::Vector<Rml::String> m_vrDeviceList;
};
