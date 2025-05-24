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

	static const std::string k_trackingSystemPropertyId;
	MikanStageTrackingSystem getTrackingSystem() const { return m_trackingSystem; }
	void setTrackingSystem(MikanStageTrackingSystem system);

	static const std::string k_originMarkerIdPropertyId;
	MikanMarkerID getOriginMarkerId() const { return m_originMarkerId; }
	void setOriginMarkerId(MikanMarkerID markerId);

	static const std::string k_originMarkerSizePropertyId;
	float getOriginMarkerSize() const { return m_originMarkerSizeMM; }
	void setOriginMarkerSize(float size);

	static const std::string k_utilityMarkerIdPropertyId;
	MikanMarkerID getUtilityMarkerId() const { return m_utilityMarkerId; }
	void setUtilityMarkerId(MikanMarkerID markerId);

	static const std::string k_utilityMarkerSizePropertyId;
	float getUtilityMarkerSize() const { return m_utilityMarkerSizeMM; }
	void setUtilityMarkerSize(float size);

protected:
	MikanStageID m_stageId = INVALID_MIKAN_ID;
	MikanStageTrackingSystem m_trackingSystem= MikanStageTrackingSystem::StaticMarker;
	MikanMarkerID m_originMarkerId = DEFAULT_ORIGIN_MARKER_ID;
	float m_originMarkerSizeMM= DEFAULT_MARKER_SIZE_MM;
	MikanMarkerID m_utilityMarkerId = DEFAULT_UTILITY_MARKER_ID;
	float m_utilityMarkerSizeMM= DEFAULT_MARKER_SIZE_MM;
};

class StageComponent final : public TransformComponent
{
public:
	StageComponent(MikanObjectWeakPtr owner);

	inline StageComponentDefinitionPtr getStageComponentDefinition() const
	{
		return std::static_pointer_cast<StageComponentDefinition>(m_definition);
	}

	// -- MikanComponent ----
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;
	virtual void init() override;
	virtual void dispose() override;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteStageFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	// -- StageComponent ----
	void deleteStage();
};