#pragma once

#include "ComponentFwd.h"
#include "ColliderQuery.h"
#include "ObjectFwd.h"
#include "MikanStageTypes.h"
#include "MikanRendererFwd.h"
#include "ProjectConfigConstants.h"
#include "SceneFwd.h"
#include "TransformComponent.h"

#include <functional>

class StageComponentDefinition : public TransformComponentDefinition
{
public:
	StageComponentDefinition()= default;
	StageComponentDefinition(
		MikanStageID sceneId,
		const std::string& componentName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanStageID getStageId() const { return m_stageId; }

	static const std::string k_trackingSystemIdPropertyId;
	MikanTrackingSystemID getTrackingSystemId() const { return m_trackingSystemId; }
	void setTrackingSystemId(MikanTrackingSystemID systemId);

protected:
	MikanStageID m_stageId = INVALID_MIKAN_ID;
	MikanTrackingSystemID m_trackingSystemId = INVALID_MIKAN_ID;
};

class StageComponent final : public TransformComponent
{
public:
	StageComponent(MikanObjectWeakPtr owner);

	inline StageComponentDefinitionConstPtr getStageComponentDefinitionConst() const
	{
		return std::static_pointer_cast<const StageComponentDefinition>(m_definition);
	}
	inline StageComponentDefinitionPtr getStageComponentDefinition()
	{
		return std::static_pointer_cast<StageComponentDefinition>(m_definition);
	}

	TrackingAPIDefinitionConstPtr getTrackingAPIDefinitionConst() const;

	// -- MikanComponent ----
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void init() override;
	virtual void dispose() override;

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_alignStageFunctionId;
	static const std::string k_deleteStageFunctionId;
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	// -- StageComponent ----
	void alignStage();
	void deleteStage();
};