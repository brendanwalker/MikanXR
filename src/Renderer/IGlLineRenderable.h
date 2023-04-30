#pragma once

#include "RendererFwd.h"

class IGlLineRenderable
{
public:
	virtual void renderLines() const = 0;
};