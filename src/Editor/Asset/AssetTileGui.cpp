#include "AssetTileGui.h"
#include "AssetReference.h"
#include "IMkTexture.h"

#include "imgui.h"

namespace AssetTileGui
{
// The tile's preview square, inset from the frame
static constexpr float k_previewSize= 100.f;
static constexpr float k_previewInset= 10.f;
// Row spacing between tiles
static constexpr float k_tileGap= 10.f;

std::string truncateTextWithEllipsis(const std::string& text, float maxWidth)
{
	if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
	{
		return text;
	}

	std::string truncated= text;
	while (!truncated.empty() && ImGui::CalcTextSize((truncated + "...").c_str()).x > maxWidth)
	{
		// Pop one codepoint (skip UTF-8 continuation bytes)
		truncated.pop_back();
		while (!truncated.empty() && ((unsigned char)truncated.back() & 0xC0) == 0x80)
		{
			truncated.pop_back();
		}
	}

	return truncated + "...";
}

TileResult beginTile(const std::string& idStr, bool bSelected)
{
	TileResult result;

	ImGui::Dummy(ImVec2(k_tileGap, k_tileHeight));
	ImGui::SameLine();
	ImGui::BeginGroup();

	// A selected tile keeps the active button tint at rest
	if (bSelected)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
	}
	result.bClicked= ImGui::Button(idStr.c_str(), ImVec2(k_tileWidth, k_tileHeight));
	if (bSelected)
	{
		ImGui::PopStyleColor();
	}

	// Read the hover state before a drag source claims the item
	result.bHovered= ImGui::IsItemHovered();
	result.bDoubleClicked= result.bHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

	return result;
}

void endTile(AssetReferencePtr assetRef, const std::string& displayName)
{
	IMkTexturePtr texture= assetRef ? assetRef->getPreviewTexture() : IMkTexturePtr();

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + k_previewInset);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (k_tileHeight - k_previewInset));

	if (texture && texture->getGlTextureId() != 0)
	{
		ImGui::Image((ImTextureID)(intptr_t)texture->getGlTextureId(), ImVec2(k_previewSize, k_previewSize));
	}
	else
	{
		// Type icon stand-in for assets with no preview image
		const ImVec2 tileMin= ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(k_previewSize, k_previewSize));

		const char* icon= assetRef ? assetRef->editorGetIcon() : "";
		ImFont* font= ImGui::GetFont();
		const float iconFontSize= ImGui::GetFontSize() * 3.f;
		const ImVec2 iconSize= font->CalcTextSizeA(iconFontSize, FLT_MAX, 0.f, icon);
		const ImVec2 iconPos(tileMin.x + (k_previewSize - iconSize.x) * 0.5f,
							 tileMin.y + (k_previewSize - iconSize.y) * 0.5f);
		ImGui::GetWindowDrawList()->AddText(font, iconFontSize, iconPos, ImGui::GetColorU32(ImGuiCol_Text), icon);
	}
	ImGui::Dummy(ImVec2(2, 1));
	ImGui::SameLine();

	const std::string clippedName= truncateTextWithEllipsis(displayName, k_previewSize + 4.f);
	ImGui::TextUnformatted(clippedName.c_str());
	if (ImGui::IsItemHovered() && clippedName != displayName)
	{
		ImGui::SetTooltip("%s", displayName.c_str());
	}

	ImGui::EndGroup();

	// Tiles flow left to right and wrap when the next one would not fit
	ImGui::SameLine();
	if (ImGui::GetContentRegionAvail().x < k_tileWidth + k_tileGap)
	{
		ImGui::NewLine();
		ImGui::NewLine();
	}
}
} // namespace AssetTileGui
