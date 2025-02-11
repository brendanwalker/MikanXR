#pragma once

#include "MkRendererFwd.h"

class IMkBindableObject
{
protected:
	virtual void bindObject(IMkState* glParentState) = 0;
	virtual bool getIsBound() const = 0;
	virtual void unbindObject() = 0;

	friend class MkScopedObjectBinding;
};
