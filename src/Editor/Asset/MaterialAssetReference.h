#pragma once

#include "AssetReference.h"
#include "LocText.h"

// The behavior every material reference shares: opening the source graph and
// dropping onto a graph as a material property and node. The compositor and
// shape subclasses carry the class name, extension, and folder of their domain.
class MaterialAssetReference : public AssetReference
{
public:
	MaterialAssetReference()= default;

	virtual const char* editorGetIcon() const override { return ICON_FK_PAINT_BRUSH; }

	virtual void setAssetPath(const std::filesystem::path& inPath) override;

	virtual void editorHandleGraphVariablesDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorHandleMainFrameDragDrop(const class NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const class NodeEditorState& editorState) override;
	// True when the .mat records the material graph it was compiled from
	virtual bool editorCanOpen() const override;
	// Opens that graph in the material editor window, creating the window when none is open
	virtual void editorOpen() override;

protected:
	virtual void rebuildPreview() override;

	// The .mat's source graph resolved against the .mat folder, empty when it has
	// none. Read once per asset path, since editorCanOpen is asked every frame.
	const std::filesystem::path& getSourceGraphPath() const;

protected:
	mutable std::filesystem::path m_sourceGraphPath;
	mutable bool m_bSourceGraphPathResolved= false;
};

enum class eMaterialDomain : int;

// The material reference class for a domain, the one its graphs register
const std::string& getMaterialAssetClassName(eMaterialDomain domain);
