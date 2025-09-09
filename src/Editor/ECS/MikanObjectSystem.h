#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "FunctionInterface.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "PropertyInterface.h"

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
	public IPropertyInterface,
	public IFunctionInterface
{
public:
	MikanObjectSystem(class ObjectSystemManager* ownerObjectSystem);
	virtual ~MikanObjectSystem();

	virtual bool init();
	virtual void dispose();
	virtual void update(float deltaSeconds);
	virtual void customRender();

	inline class ObjectSystemManager* getOwnerObjectSystemManager() const { return m_ownerObjectSystemManager; }
	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const {
		return MikanObjectSystemDefinitionConstPtr();
	}
	virtual MikanObjectSystemDefinitionPtr getObjectSystemConfig() {
		return std::const_pointer_cast<MikanObjectSystemDefinition>(getObjectSystemConfigConst());
	}

	MikanObjectPtr newObject();
	void deleteObject(MikanObjectPtr objectPtr);
	void deleteAllObjects();
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) { }
	inline const MikanObjectList& getObjectList() const { return m_objects; }

	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectPtr)> OnObjectInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectConstPtr)> OnObjectDisposed;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentPtr)> OnComponentInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentConstPtr)> OnComponentDisposed;

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames) {}
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override {}
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override { return false; }
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override { return false; }
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override { return false; }
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override { return false; }

	// -- IFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames) {}
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override {}
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override { return false; }
	virtual bool invokeFunction(const std::string& propertyName) override { return false; }

protected:
	void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	class ObjectSystemManager* m_ownerObjectSystemManager = nullptr;
	MikanObjectList m_objects;

	MulticastDelegate<void(float deltaSeconds)> onUpdate;
	MulticastDelegate<void()> onCustomRender;

	friend class MikanComponent;
};