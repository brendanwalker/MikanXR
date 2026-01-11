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
	StageComponentDefinition(MikanStageID sceneId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanStageID getStageId() const { return m_stageId; }

	static const std::string k_trackingVolumeIdPropertyId;
	MikanTrackingVolumeID getTrackingVolumeId() const { return m_trackingVolumeId; }
	void setTrackingVolumeId(MikanTrackingVolumeID volumeId);

protected:
	MikanStageID m_stageId = INVALID_MIKAN_ID;
	MikanTrackingVolumeID m_trackingVolumeId = INVALID_MIKAN_ID;
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

	inline static const std::string k_componentClassName = "StageComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	TrackingVolumeComponentConstPtr getTrackingVolumeConst() const;
	TrackingVolumeDefinitionConstPtr getTrackingVolumeDefinitionConst() const;

	// -- MikanComponent ----
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void init() override;
	virtual void dispose() override;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteStageFunctionId;
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	// -- StageComponent ----
	MikanStageID getStageId() const;
	void setTrackingVolumeId(MikanTrackingVolumeID volumeId);
	void deleteStage();
};