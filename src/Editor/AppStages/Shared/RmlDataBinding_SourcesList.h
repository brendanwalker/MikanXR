#pragma once

#include "Shared\RmlDataBinding.h"

class RmlDataBinding_SourcesList : public RmlDataBinding
{
public:
	RmlDataBinding_SourcesList()= default;
	virtual ~RmlDataBinding_SourcesList();

	virtual bool init(Rml::DataModelConstructor constructor) override;
	virtual void dispose() override;

	const Rml::Vector<Rml::String>& getVideoDeviceList() const { return m_sourcePathList; }

protected:
	void rebuildSourcesList();

private:
	Rml::Vector<Rml::String> m_sourcePathList;
};
