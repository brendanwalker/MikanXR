#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "IMkBindableObject.h"

#include <string>

class MIKAN_RENDERER_CLASS MkScopedObjectBinding
{
public:
	MkScopedObjectBinding() = default;
	MkScopedObjectBinding(
		IMkState* parentMkState, 
		const std::string& scopeName, 
		IMkBindableObjectPtr bindableObject);
	virtual ~MkScopedObjectBinding();

	IMkBindableObjectPtr getBoundObject() const;

	template <class t_bindable_object>
	inline std::shared_ptr<t_bindable_object> getTypedBoundObject() const
	{
		return std::static_pointer_cast<t_bindable_object>(getBoundObject());
	}

	operator bool() const;
	IMkState* getMkState();

private:
	struct MkScopedObjectBindingData* m_data;
}; 