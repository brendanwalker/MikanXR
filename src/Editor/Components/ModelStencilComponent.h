#pragma once

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
	const std::filesystem::path& getModelPath() const { return m_modelPath; }
	void setModelPath(const std::filesystem::path& path, bool bForceDirty= false);

	static const std::string k_modelStencilIsDepthMeshPropertyId;
	bool getIsDepthMesh() const { return m_bIsDepthMesh; }
	void setIsDepthMesh(bool isDepthMesh);

	bool hasValidDepthMesh() const;

private:
	std::filesystem::path m_modelPath;
	std::filesystem::path m_texturePath;
	bool m_bIsDepthMesh= false;
};

class ModelStencilComponent : public StencilComponent
{
public:
	ModelStencilComponent(MikanObjectWeakPtr owner);
	virtual void init() override;
	virtual void customRender() override;
	virtual void dispose() override;

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

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_alignStencilFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void alignStencil();

protected:
	void updateWireframeMeshColor();
	void onStencilDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	SelectionComponentWeakPtr m_selectionComponentWeakPtr;
	std::vector<IMkStaticMeshInstancePtr> m_wireframeMeshes;
	std::vector<SceneComponentPtr> m_meshComponents;
	std::vector<StaticMeshComponentPtr> m_triMeshComponents;
	std::vector<MeshColliderComponentPtr> m_colliderComponents;
	bool m_bIsHovered= false;
	bool m_bIsSelected= false;
	bool m_bIsTransformGizmoBound= false;
};
