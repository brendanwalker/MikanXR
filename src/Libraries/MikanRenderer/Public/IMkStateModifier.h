#pragma once

#include <string>
#include <memory>

class IMkStateModifier
{
public:
	virtual int getOwnerStateStackDepth() const = 0;
	virtual const std::string& getModifierID() const = 0;
	virtual void apply(std::shared_ptr<IMkStateModifier> parentModifier) = 0;
	virtual void revert() = 0;
};
using MkStateModifierPtr = std::shared_ptr<IMkStateModifier>;
