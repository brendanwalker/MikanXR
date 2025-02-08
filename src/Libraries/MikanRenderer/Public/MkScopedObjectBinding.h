#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "IMkBindableObject.h"

#include <memory>
#include <string>

class MIKAN_RENDERER_CLASS MkScopedObjectBinding
{
public:
	MkScopedObjectBinding() = default;
	MkScopedObjectBinding(class GlState& parentGLState, const std::string& scopeName, IMkBindableObjectPtr bindableObject);
	virtual ~MkScopedObjectBinding();

	template <class t_bindable_object>
	inline std::shared_ptr<t_bindable_object> getBoundObject() const
	{
		return std::static_pointer_cast<t_bindable_object>(m_boundObject);
	}

	inline operator bool() const { return m_boundObject->getIsBound(); }
	inline GlState& getGlState() { return m_glState; }

private:
	IMkBindableObjectPtr m_boundObject;
	class GlState& m_glState;
}; 