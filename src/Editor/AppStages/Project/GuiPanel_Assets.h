#pragma once

#include "AssetFwd.h"
#include "IMkGuiStyle.h"
#include "Shared/GuiPanel.h"

#include <string>

struct ProjectAssetEntry;
struct ProjectAssetFolderDesc;

// The project's asset folders as a folder list beside a grid of asset tiles.
// The only place assets are imported into the project or deleted from it.
// Tiles are drag sources for the component property rows that take an asset.
class GuiPanel_Assets : public GuiPanel
{
public:
	GuiPanel_Assets(AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}
	virtual ~GuiPanel_Assets();

	bool init(class ProjectGuiPanelContext* context);
	virtual void onGui() override;

	// Makes the entry's folder current, selects the entry and scrolls it into
	// view. False when the catalog holds no such entry.
	bool selectEntry(const std::string& folderId, const std::string& storedPath);
	const ProjectAssetEntry* getSelectedEntry() const;
	const std::string& getCurrentFolderId() const { return m_currentFolderId; }
	void setCurrentFolder(const std::string& folderId);

private:
	class ProjectAssetCatalog* getCatalog() const;
	void onCatalogChanged();

	void renderFolderList();
	void renderToolbar(const ProjectAssetFolderDesc& desc);
	void renderTileGrid(const ProjectAssetFolderDesc& desc);
	void importFromFileDialog(const ProjectAssetFolderDesc& desc, AssetReferenceFactoryPtr factory);
	void requestDeleteEntry(const ProjectAssetEntry& entry);

	class ProjectGuiPanelContext* m_context= nullptr;
	std::string m_currentFolderId;
	// The selected entry's key(), empty when nothing is selected. The key is kept
	// rather than the entry, which a catalog refresh replaces.
	std::string m_selectedKey;
	bool m_bScrollToSelection= false;
	MkGuiStyleConstPtr m_contextMenuStyle;
};
