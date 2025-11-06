#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "RmlFunctionInterface.h"
#include "RmlPropertyInterface.h"

#include <vector>

using MikanObjectList = std::vector<MikanObjectPtr>;

class MikanObjectSystemDefinition : public CommonConfig
{
public:
	MikanObjectSystemDefinition(const std::string& configName)
		: CommonConfig(configName)
	{
	}
};

class MikanObjectSystem : 
	public std::enable_shared_from_this<MikanObjectSystem>,
	public IRmlPropertyInterface,
	public IRmlFunctionInterface
{
public:
	MikanObjectSystem(class ProjectManager* ownerObjectSystem);
	virtual ~MikanObjectSystem();

	virtual bool init();
	virtual void dispose();
	virtual void update(float deltaSeconds);
	virtual void customRender();

	inline class ProjectManager* getOwnerProjectManager() const { return m_ownerObjectSystemManager; }
	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const {
		return MikanObjectSystemDefinitionConstPtr();
	}
	virtual MikanObjectSystemDefinitionPtr getObjectSystemConfig() {
		return std::const_pointer_cast<MikanObjectSystemDefinition>(getObjectSystemConfigConst());
	}
	ProjectConfigPtr getProjectConfig() const;

	MikanObjectPtr newObject();
	void deleteObject(MikanObjectPtr objectPtr);
	void deleteAllObjects();
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) { }
	inline const MikanObjectList& getObjectList() const { return m_objects; }

	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectPtr)> OnObjectInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectConstPtr)> OnObjectDisposed;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentPtr)> OnComponentInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentConstPtr)> OnComponentDisposed;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors) {}
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override { return false; }
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override { return false; }

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors) {}
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)  override { return false; }

protected:
	class ProjectManager* m_ownerObjectSystemManager = nullptr;

	MikanObjectList m_objects;

	MulticastDelegate<void(float deltaSeconds)> onUpdate;
	MulticastDelegate<void()> onCustomRender;

	friend class MikanComponent;
};