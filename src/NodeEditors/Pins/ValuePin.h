#pragma once

#include "NodePin.h"

#include <array>

typedef unsigned int ImU32;

class ValuePin : public NodePin
{
public:
	ValuePin() = default;

	// Shared value pin functions
	virtual float editorComputeInputWidth() const;
	virtual ImNodesPinShape editorRenderBeginPin(float alpha) override;

	// Override these in derived pin types
	inline static const std::string k_pinClassName = "ValuePin";
	virtual std::string getClassName() const override { return k_pinClassName; }
	virtual const ImU32 editorValuePinColor(float alpha) const;
};

template<typename t_data_type>
class TypedValuePin : public ValuePin
{
public:
	TypedValuePin() = default;

	t_data_type getValue() const { return m_value; }
	void setValue(t_data_type inValue) { m_value = inValue; }

	virtual size_t getDataSize() const { return sizeof(t_data_type); }
	virtual void copyValueFromSourcePin() override
	{
		auto sourcePin = std::dynamic_pointer_cast< TypedValuePin<t_data_type> >(getConnectedSourcePin());

		if (sourcePin)
		{
			setValue(sourcePin->getValue());
		}
	}

protected:
	t_data_type m_value;
};