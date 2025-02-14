#pragma once

#include "MkRendererFwd.h"

#include <string>
#include <memory>

class IMkStateModifier
{
public:
	virtual int getOwnerStateStackDepth() const = 0;
	virtual const std::string& getModifierID() const = 0;
	virtual void apply(IMkStateModifierConstPtr parentModifier) = 0;
	virtual void revert() = 0;
};