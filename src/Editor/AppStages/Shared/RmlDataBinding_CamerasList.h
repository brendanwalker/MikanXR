#pragma once

#include "CommonConfigFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared\RmlDataBinding.h"

class RmlDataBinding_CamerasList : public RmlDataBinding
{
public:
	RmlDataBinding_CamerasList()= default;
	virtual ~RmlDataBinding_CamerasList();

	virtual bool init(Rml::DataModelConstructor constructor) override;
	virtual void dispose() override;

	const Rml::Vector<int>& getCameraIdList() const { return m_cameraIdList; }

protected:
	void onObjectInitialized(MikanObjectSystemPtr objectSystemPtr, MikanObjectPtr objectPtr);
	void onObjectDisposed(MikanObjectSystemPtr objectSystemPtr, MikanObjectConstPtr objectPtr);
	void onCameraSystemConfigMarkedDirty(
		CommonConfigPtr configPtr, 
		const ConfigPropertyChangeSet& changedPropertySet);
	void rebuildCameraIdList();

private:
	Rml::Vector<int> m_cameraIdList;
};
