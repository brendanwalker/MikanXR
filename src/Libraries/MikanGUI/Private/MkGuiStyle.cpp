#include "MkGuiStyle.h"

const MkGuiStyleTextureEntry* MkGuiStyle::findTexture(const std::string& name) const
{
	auto it = m_textures.find(name);
	return (it != m_textures.end()) ? &it->second : nullptr;
}
