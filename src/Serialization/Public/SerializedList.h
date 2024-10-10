#pragma once

#include "MikanExport.h"

#include <vector>

#ifdef ENABLE_REFLECTION
#include "SerializedList.rfkh.h"
#endif

namespace Serialization NAMESPACE()
{
	template <typename T>
	class CLASS() List : public std::vector<T>
	{
		METHOD()
		std::size_t size() const noexcept
		{
			return std::vector<T>::size();
		}

		METHOD()
		void resize(std::size_t newSize) const noexcept
		{
			return std::vector<T>::resize(newSize);
		}

		METHOD()
		const void* getRawElement(const std::size_t index) const
		{
			return &at(index);
		}

	#ifdef ENABLE_REFLECTION
	Serialization_List_GENERATED
	#endif
	};
}

#ifdef ENABLE_REFLECTION
File_SerializedList_GENERATED
#endif