#pragma once

#include <memory>

class IGlBindableObject
{
protected:
	virtual void bindObject(class GlState& glParentState) = 0;
	virtual bool getIsBound() const = 0;
	virtual void unbindObject() = 0;

	friend class GlScopedObjectBinding;
};
using GlBindableObjectPtr = std::shared_ptr<IGlBindableObject>;
using GlBindableObjectConstPtr = std::shared_ptr<const IGlBindableObject>;
