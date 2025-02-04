#pragma once

#include <string>
#include <memory>

class IGlStateModifier
{
public:
	virtual int getOwnerStateStackDepth() const { return -1; }
	inline static const std::string k_modifierID = "<INVALID>";
	virtual const std::string& getModifierID() const { return k_modifierID; }
	virtual void apply(std::shared_ptr<IGlStateModifier> parentModifier) {}
	virtual void revert() {}
};
using GlStateModifierPtr = std::shared_ptr<IGlStateModifier>;
