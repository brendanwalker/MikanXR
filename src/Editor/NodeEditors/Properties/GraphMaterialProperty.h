#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "GraphProperty.h"
#include "MaterialCompiler/MaterialDomain.h"

class GraphMaterialPropertyConfig : public GraphPropertyConfig
{
public:
	GraphMaterialPropertyConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	int assetRefIndex;
};

class GraphMaterialProperty : public GraphProperty
{
public:
	GraphMaterialProperty()= default;
	virtual ~GraphMaterialProperty();

	inline static const std::string k_propertyClassName= "GraphMaterialProperty";
	virtual std::string getClassName() const override { return k_propertyClassName; }

	virtual bool loadFromConfig(GraphPropertyConfigConstPtr propConfig, const NodeGraphConfig& graphConfig) override;
	virtual void saveToConfig(GraphPropertyConfigPtr config) const override;

	void setMaterialAssetReference(MaterialAssetReferencePtr inAssetRef);
	inline MaterialAssetReferencePtr getMaterialAssetReference() const { return m_materialAssetRef; }

	inline MkMaterialConstPtr getMaterialResource() const { return m_materialResource; }

	// The .mat's domain, inferred from its vertex attributes when the file does
	// not name one. INVALID when the material failed to load or its attributes
	// name no position.
	inline eMaterialDomain getDomain() const { return m_domain; }
	// Whether the material may feed a consumer of the given domain. An INVALID
	// domain passes, so a legacy .mat that cannot be classified still connects.
	bool isCompatibleWithDomain(eMaterialDomain domain) const;

	// The input pin class a consumer node exposes a material uniform of the
	// given data type through, empty when no pin carries that type
	static const std::string& getUniformPinClassName(eUniformDataType dataType);
	// Seed a freshly created dynamic float pin with the material's default for its uniform
	static void initDynamicPinFromMaterialDefault(NodePinPtr pin, MkMaterialConstPtr material);

	// Opens the material's source graph in the material editor. Returns false,
	// after logging why, when the .mat names no source graph.
	bool editorOpenSourceGraph() const;

	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	virtual std::string editorGetTitle() const override { return "Material"; }

protected:
	// The owner window's shader cache, null when the graph has no window (headless loads)
	class MikanShaderCache* getShaderCache() const;
	void unbindMaterialReloadListener();
	void onMaterialReloaded(MkMaterialPtr material);

protected:
	MaterialAssetReferencePtr m_materialAssetRef;
	MkMaterialConstPtr m_materialResource;
	eMaterialDomain m_domain= eMaterialDomain::INVALID;
	// The cache OnMaterialReloaded is bound on while a resource is held
	class MikanShaderCache* m_listenedShaderCache= nullptr;
};

using GraphMaterialPropertyFactory= TypedGraphPropertyFactory<GraphMaterialProperty, GraphMaterialPropertyConfig>;
