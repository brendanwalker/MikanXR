#pragma once

#include "RendererFwd.h"

#include <memory>
#include <filesystem>

class AssetReference : public std::enable_shared_from_this<AssetReference>
{
public:
	AssetReference()= default;
	virtual ~AssetReference();

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

	virtual std::string getAssetTypeName() const { return "Asset"; }
	virtual char const* getFileDialogTitle() const { return "Load Asset"; }
	virtual char const* getDefaultPath() const { return ""; }
	virtual char const* const* getFilterPatterns() const { return nullptr; }
	virtual int getFilterPatternCount() const { return 0; }
	virtual char const* getFilterDescription() const { return ""; }

	virtual std::shared_ptr<AssetReference> createAssetReference(
		const class NodeEditorState* editorState,
		const std::filesystem::path& inAssetPath) const;

	template <class t_factory_class>
	static std::shared_ptr<t_factory_class> create()
	{
		return std::make_shared<t_factory_class>();
	}
};