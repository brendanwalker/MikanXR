#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IEntityAccessor.h"
#include "MulticastDelegate.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectManager.h"

#include <vector>

using MikanObjectList = std::vector<MikanObjectPtr>;
using MikanPropertyDatabasePtr = std::shared_ptr<class MikanPropertyDatabase>;

class MikanObjectSystemDefinition : public CommonConfig
{
public:
	MikanObjectSystemDefinition(const std::string& configName)
		: CommonConfig(configName)
	{
	}

	MikanObjectSystemPtr getOwnerSystem() const { return m_ownerSystem.lock(); }
	void setOwnerSystem(MikanObjectSystemPtr ownerSystem) { m_ownerSystem = ownerSystem; }

protected:
	MikanObjectSystemWeakPtr m_ownerSystem;
};

class MikanObjectSystem : 
	public std::enable_shared_from_this<MikanObjectSystem>,
	public IEntityAccessor
{
public:
	MikanObjectSystem(ProjectManagerPtr ownerObjectSystem);
	virtual ~MikanObjectSystem();

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr);
	virtual void dispose();
	virtual void update(float deltaSeconds);
	virtual void customRender();

	inline static const std::string k_objectSystemClassName = "MikanObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline ProjectManagerPtr getOwnerProjectManager() const { return m_ownerObjectSystemManager.lock(); }
	ProjectConfigPtr getProjectConfig() const;

	template <class t_object_system_type>
	std::shared_ptr<t_object_system_type> getObjectSystemOfType() const
	{
		return getOwnerProjectManager()->getSystemOfType<t_object_system_type>();
	}


	inline MikanObjectSystemDefinitionConstPtr getDefinitionConst() const {
		return m_definitionWeakPtr.lock();
	}
	inline MikanObjectSystemDefinitionPtr getDefinition() {
		return m_definitionWeakPtr.lock();
	}

	virtual MikanComponentPtr getComponentById(int componentId) const = 0;
	virtual bool getComponentIdList(const std::string& componentClassName, std::vector<int>& outComponentIdList) const = 0;

	MikanObjectPtr newObject();
	void deleteObject(MikanObjectPtr objectPtr);
	void deleteAllObjects();
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) { }
	inline const MikanObjectList& getObjectList() const { return m_objects; }

	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectPtr)> OnObjectInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanObjectConstPtr)> OnObjectDisposed;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentPtr)> OnComponentInitialized;
	MulticastDelegate<void(MikanObjectSystemPtr, MikanComponentConstPtr)> OnComponentDisposed;
	
	// -- IEntityAccessor ----
	virtual CommonConfigPtr getEntityConfig() override { return getDefinition(); }
	virtual rfk::Struct const* getClientAPIValuesStructType() const;

	// -- IPropertyInterface ----
	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase);
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors) {}
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override { return false; }
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override { return false; }

	// -- IFunctionInterface ----
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors) {}
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc)  override { return false; }
		
protected:
	ProjectManagerWeakPtr m_ownerObjectSystemManager;
	MikanObjectSystemDefinitionWeakPtr m_definitionWeakPtr;

	MikanObjectList m_objects;

	MulticastDelegate<void(float deltaSeconds)> onUpdate;
	MulticastDelegate<void()> onCustomRender;

	friend class MikanComponent;
};