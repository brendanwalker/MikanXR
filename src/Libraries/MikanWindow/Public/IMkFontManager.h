#pragma once

#include "MkWindowExport.h"
#include "IMkTextRenderer.h"

#include <memory>

using IMkFontManagerPtr= std::shared_ptr<IMkFontManager>;

MIKAN_WINDOW_FUNC(IMkFontManagerPtr) createMkFontManager();