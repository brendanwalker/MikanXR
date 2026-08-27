#pragma once

#include "MkGuiExport.h"

enum class MkCanvasPinDirection : int
{
	Input,
	Output,
};

// One pin on the current node; content between construction and destruction
// is the pin's hit area (icon plus label)
class MIKAN_GUI_CLASS MkCanvasScopedPin
{
public:
	MkCanvasScopedPin(int pinId, MkCanvasPinDirection direction);
	~MkCanvasScopedPin();

	MkCanvasScopedPin(const MkCanvasScopedPin&)= delete;
	MkCanvasScopedPin& operator=(const MkCanvasScopedPin&)= delete;
};
