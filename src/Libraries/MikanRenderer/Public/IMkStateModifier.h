#pragma once

#include <string>
#include <memory>

class IMkStateModifier
{
public:
	virtual int getOwnerStateStackDepth() const { return -1; }
	inline static const std::string k_modifierID = "<INVALID>";
	virtual const std::string& getModifierID() const { return k_modifierID; }
	virtual void apply(std::shared_ptr<IMkStateModifier> parentModifier) {}
	virtual void revert() {}
};
using MkStateModifierPtr = std::shared_ptr<IMkStateModifier>;
