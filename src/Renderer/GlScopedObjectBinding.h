#pragma once

#include "IGlBindableObject.h"

class GlScopedObjectBinding
{
public:
	GlScopedObjectBinding() = default;
	GlScopedObjectBinding(class GlState& parentGLState, GlBindableObjectPtr bindableObject);
	virtual ~GlScopedObjectBinding();

	template <class t_bindable_object>
	inline std::shared_ptr<t_bindable_object> getBoundObject() const
	{
		return std::static_pointer_cast<t_bindable_object>(m_boundObject);
	}

	inline operator bool() const { return m_boundObject->getIsBound(); }
	inline GlState& getGlState() { return m_glState; }

private:
	GlBindableObjectPtr m_boundObject;
	class GlState& m_glState;
}; 