#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "IconsForkAwesome.h"
#include "MikanRendererFwd.h"

#include <memory>
#include <filesystem>

class AssetReferenceConfig : public CommonConfig
{
public:
	AssetReferenceConfig()
		: CommonConfig()
	{
	}
	AssetReferenceConfig(const std::string& nodeName)
		: CommonConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	bool isValid() const;

	std::string className;
	std::string assetPath;
};

class AssetReference : public std::enable_shared_from_this<AssetReference>
{
public:
	AssetReference()= default;
	virtual ~AssetReference();

	inline static const std::string k_assetClassName= "AssetReference";
	virtual std::string getClassName() const { return k_assetClassName; }

	virtual bool loadFromConfig(AssetReferenceConfigConstPtr config);
	virtual void saveToConfig(AssetReferenceConfigPtr config) const;

	virtual std::string getAssetTypeName() const { return "Asset"; }
	// ForkAwesome glyph shown when the asset has no preview texture
	virtual const char* editorGetIcon() const { return ICON_FK_FILE_O; }
	inline IMkTexturePtr getPreviewTexture() const { return m_previewTexture; }

	// The stored form of the path: forward slashes, relative to the project when the
	// file sits under it. setAssetPath normalizes whatever it is given into this form.
	const std::filesystem::path& getInternalAssetPath() const;
	const std::filesystem::path getResolvedAssetPath() const;
	virtual void setAssetPath(const std::filesystem::path& inPath);

	std::string getShortName() const;

	bool isEmpty() const;

	virtual void editorHandleGraphVariablesDragDrop(const class NodeEditorState& editorState) {}
	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) {}
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) {}
	// Whether double-clicking the asset opens it in a dedicated editor window
	virtual bool editorCanOpen() const { return false; }
	virtual void editorOpen() {}

protected:
	virtual void rebuildPreview() {}

protected:
	std::filesystem::path m_assetPath;
	IMkTexturePtr m_previewTexture;
};

class AssetReferenceFactory
{
public:
	AssetReferenceFactory()= default;

	inline std::string getAssetRefClassName() const { return m_defaultAssetRefObject->getClassName(); }

	virtual std::string getAssetTypeName() const { return "Asset"; }
	virtual char const* getFileDialogTitle() const { return "Load Asset"; }
	virtual char const* getDefaultPath() const { return m_defaultPath.c_str(); }
	virtual char const* const* getFilterPatterns() const { return nullptr; }
	virtual int getFilterPatternCount() const { return 0; }
	virtual char const* getFilterDescription() const { return ""; }

	virtual AssetReferenceConfigPtr allocateAssetReferenceConfig() const;
	virtual AssetReferencePtr allocateAssetReference() const;

	// The prototype instance the factory was created with, for its icon and type name
	inline AssetReferencePtr getDefaultAssetReference() const { return m_defaultAssetRefObject; }

	// Whether the file's extension matches one of the factory's "*.ext" dialog patterns
	bool matchesFilterPatterns(const std::filesystem::path& path) const;

	template <class t_factory_class>
	static std::shared_ptr<t_factory_class> createFactory()
	{
		auto factory= std::make_shared<t_factory_class>();

		factory->m_defaultAssetRefObject= factory->allocateAssetReference();

		return factory;
	}

	virtual bool editorCanCreate() const { return false; }

protected:
	AssetReferencePtr m_defaultAssetRefObject;
	std::string m_defaultPath;
};

template <class t_assetref_class, class t_assetref_config_class>
class TypedAssetReferenceFactory : public AssetReferenceFactory
{
public:
	TypedAssetReferenceFactory()= default;

	virtual AssetReferenceConfigPtr allocateAssetReferenceConfig() const override
	{
		AssetReferenceConfigPtr configPtr= std::make_shared<t_assetref_config_class>();

		configPtr->className= t_assetref_class::k_assetClassName;

		return configPtr;
	}

	virtual AssetReferencePtr allocateAssetReference() const override { return std::make_shared<t_assetref_class>(); }
};