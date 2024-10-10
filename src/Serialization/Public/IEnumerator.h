#pragma once

#include "MikanExport.h"

#ifdef ENABLE_REFLECTION
#include "IEnumerator.rfkh.h"
#endif

namespace Serialization NAMESPACE()
{
	class IEnumerator
	{
	public:
		virtual bool MoveNext() = 0;
		virtual void Reset() = 0;
		virtual void* Current() = 0;
	};

	class CLASS() IEnumerable
	{
	public:
		METHOD()
		virtual IEnumerator	GetEnumerator() = 0;

		#ifdef ENABLE_REFLECTION
		Serialization_IEnumerable_GENERATED
		#endif
	};
}

#ifdef ENABLE_REFLECTION
File_IEnumerator_GENERATED
#endif