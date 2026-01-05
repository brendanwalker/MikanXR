#pragma once

#include "AssetFwd.h"
#include "ColliderQuery.h"
#include "ComponentFwd.h"
#include "MikanStencilTypes.h"
#include "MikanRendererFwd.h"
#include "StencilComponent.h"
#include "Transform.h"

#include <memory>
#include <string>
#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class ModelStencilDefinition : public StencilComponentDefinition
{
public:
	ModelStencilDefinition();
	ModelStencilDefinition(const MikanStencilModelInfo& modelInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	MikanStencilModelInfo getModelInfo() const;

	static const std::string k_modelStencilObjPathPropertyId;
	bool hasModelPath() const;
	const std::filesystem::path getModelPath() const;
	void setModelPath(const std::filesystem::path& path, bool bForceDirty= false);

private:
	AssetReferenceConfigPtr m_modelAssetRefConfig;
};

class ModelStencilComponent : public StencilComponent
{
public:
	ModelStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;
	virtual void dispose() override;

	inline static const std::string k_componentClassName = "ModelStencilComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline ModelStencilDefinitionPtr getModelStencilDefinition() const { 
		return std::static_pointer_cast<ModelStencilDefinition>(m_definition); 
	}
	inline const std::vector<StaticMeshComponentPtr>& getTriangulatedMeshes() const { 
		return m_triMeshComponents; 
	}
	inline const std::vector<IMkStaticMeshInstancePtr>& getWireframeMeshes() const { 
		return m_wireframeMeshes; 
	}
	inline const std::vector<MeshColliderComponentPtr>& getColliderComponents() const
	{
		return m_colliderComponents;
	}

	void setRenderStencilsFlag(bool flag);
	void setModelPath(const std::filesystem::path& path);
	void disposeMeshComponents();
	void rebuildMeshComponents();
	void extractRenderGeometry(MikanStencilModelRenderGeometry& outRenderGeometry);

	// Selection Events
	void onInteractionRayOverlapEnter(const ColliderRaycastHitResult& hitResult);
	void onInteractionRayOverlapExit(const ColliderRaycastHitResult& hitResult);
	void onInteractionSelected();
	void onInteractionUnselected();
	void onTransformGizmoBound();
	void onTransformGizmoUnbound();

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(PropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(PropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_addNewModelFunctionId;
	static const std::string k_removeModelFunctionId;
	static const std::string k_alignStencilFunctionId;
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void addNewModel();
	void removeModel();
	void alignStencil();

	// -- Lua Binding ----
	static void bindLuaFunctions(struct lua_State* L);

protected:
	void updateWireframeMeshColor();
	void onStencilDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	AssetReferencePtr m_modelAssetRef;
	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::vector<IMkStaticMeshInstancePtr> m_wireframeMeshes;
	std::vector<TransformComponentPtr> m_meshComponents;
	std::vector<StaticMeshComponentPtr> m_triMeshComponents;
	std::vector<MeshColliderComponentPtr> m_colliderComponents;
	bool m_bIsHovered= false;
	bool m_bIsSelected= false;
	bool m_bIsTransformGizmoBound= false;
};
