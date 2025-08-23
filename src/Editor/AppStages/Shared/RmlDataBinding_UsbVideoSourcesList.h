#pragma once

#include "Shared\RmlDataBinding.h"

class RmlDataBinding_UsbVideoSourcesList : public RmlDataBinding
{
public:
	RmlDataBinding_UsbVideoSourcesList()= default;
	virtual ~RmlDataBinding_UsbVideoSourcesList();

	virtual bool init(Rml::DataModelConstructor constructor) override;
	virtual void dispose() override;

	const Rml::Vector<Rml::String>& getVideoDeviceList() const { return m_sourcePathList; }

protected:
	void rebuildSourcesList();

private:
	Rml::Vector<Rml::String> m_sourcePathList;
};
