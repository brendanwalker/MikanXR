#pragma once

class IMkBindableObject
{
protected:
	virtual void bindObject(class GlState& glParentState) = 0;
	virtual bool getIsBound() const = 0;
	virtual void unbindObject() = 0;

	friend class MkScopedObjectBinding;
};
