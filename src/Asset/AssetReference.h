#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "RendererFwd.h"

#include <memory>
#include <filesystem>

class AssetReferenceConfig : public CommonConfig
{
public:
	AssetReferenceConfig() : CommonConfig() {}
	AssetReferenceConfig(const std::string& nodeName) : CommonConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string className;
	std::string assetPath;
};

class AssetReference : public std::enable_shared_from_this<AssetReference>
{
public:
	AssetReference()= default;
	virtual ~AssetReference();

	virtual bool loadFromConfig(const class AssetReferenceConfig& config);
	virtual void saveToConfig(class AssetReferenceConfig& config) const;

	virtual std::string getAssetTypeName() const { return "Asset"; }
	inline GlTexturePtr getPreviewTexture() const { return m_previewTexture; }

	inline const std::filesystem::path& getAssetPath() const { return m_assetPath; }
	virtual void setAssetPath(const std::filesystem::path& inPath);

	std::string getShortName() const;

	bool isValid() const;

	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) {}
	virtual void editorHandleDragDrop(const class NodeEditorState& editorState) {}

protected:
	virtual void rebuildPreview() {}

protected:
	std::filesystem::path m_assetPath;
	GlTexturePtr m_previewTexture;
};

class AssetReferenceFactory
{
public:
	AssetReferenceFactory() = default;

	inline std::string getAssetRefClassName() const { return typeid(m_defaultAssetRefObject.get()).name(); }

	virtual std::string getAssetTypeName() const { return "Asset"; }
	virtual char const* getFileDialogTitle() const { return "Load Asset"; }
	virtual char const* getDefaultPath() const { return ""; }
	virtual char const* const* getFilterPatterns() const { return nullptr; }
	virtual int getFilterPatternCount() const { return 0; }
	virtual char const* getFilterDescription() const { return ""; }

	virtual std::shared_ptr<AssetReference> createAssetReference(
		const class NodeEditorState* editorState,
		const std::filesystem::path& inAssetPath) const;

	virtual AssetReferenceConfigPtr createAssetReferenceConfig() const
	{
		return std::make_shared<AssetReferenceConfig>();
	}

	template <class t_factory_class>
	static std::shared_ptr<t_factory_class> createFactory()
	{
		auto factory= std::make_shared<t_factory_class>();

		factory->m_defaultAssetRefObject= factory->createAssetReference(nullptr, std::filesystem::path());

		return factory;
	}

protected:
	AssetReferencePtr m_defaultAssetRefObject;
};