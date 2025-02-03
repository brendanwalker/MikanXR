#pragma once

#include "RendererFwd.h"
#include "ValuePin.h"

class BoolPin : public TypedValuePin<bool>
{
public:
	BoolPin() = default;

	inline static const std::string k_pinClassName = "BoolPin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual const ImU32 editorValuePinColor(float alpha) const;
};