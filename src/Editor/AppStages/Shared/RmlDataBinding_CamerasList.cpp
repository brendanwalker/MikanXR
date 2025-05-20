#include "RmlDataBinding_CamerasList.h"
#include "MathUtility.h"
#include "MikanComponent.h"
#include "MulticastDelegate.h"
#include "CameraObjectSystem.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_CamerasList::~RmlDataBinding_CamerasList()
{
	dispose();
}

bool RmlDataBinding_CamerasList::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	// Start listening for camera changes
	CameraObjectSystemPtr sceneObjectSystem = CameraObjectSystem::getSystem();
	CameraObjectSystemConfigPtr sceneSystemConfig= sceneObjectSystem->getCameraSystemConfig();
	sceneObjectSystem->OnObjectInitialized +=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onObjectInitialized);
	sceneObjectSystem->OnObjectDisposed +=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onObjectDisposed);
	sceneSystemConfig->OnMarkedDirty +=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onCameraSystemConfigMarkedDirty);

	// Register Data Model Fields
	constructor.Bind("cameras_list", &m_cameraIdList);

	// Fill in the data model
	rebuildCameraIdList();

	return true;
}

void RmlDataBinding_CamerasList::dispose()
{
	// Stop listening for camera changes
	CameraObjectSystemPtr sceneObjectSystem = CameraObjectSystem::getSystem();
	CameraObjectSystemConfigPtr sceneSystemConfig = sceneObjectSystem->getCameraSystemConfig();
	sceneObjectSystem->OnObjectInitialized -=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onObjectInitialized);
	sceneObjectSystem->OnObjectDisposed -=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onObjectDisposed);
	sceneSystemConfig->OnMarkedDirty -=
		MakeDelegate(this, &RmlDataBinding_CamerasList::onCameraSystemConfigMarkedDirty);

	RmlDataBinding::dispose();
}

void RmlDataBinding_CamerasList::onObjectInitialized(
	MikanObjectSystemPtr objectSystemPtr,
	MikanObjectPtr objectPtr)
{
	rebuildCameraIdList();
}

void RmlDataBinding_CamerasList::onObjectDisposed(
	MikanObjectSystemPtr objectSystemPtr,
	MikanObjectConstPtr objectPtr)
{
	rebuildCameraIdList();
}

void RmlDataBinding_CamerasList::onCameraSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		rebuildCameraIdList();
	}
}

void RmlDataBinding_CamerasList::rebuildCameraIdList()
{
	m_cameraIdList.clear();
	for (const auto& kvpair : CameraObjectSystem::getSystem()->getCameraMap())
	{
		const MikanCameraID cameraId= kvpair.first;

		m_cameraIdList.push_back(cameraId);
	}

	m_modelHandle.DirtyVariable("cameras_list");
}