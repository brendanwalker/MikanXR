#pragma once


#include <vector>

#ifndef KODGEN_PARSING
#include "SerializableList.rfkh.h"
#endif

namespace Serialization NAMESPACE()
{
	// Turns out that std::vector<bool> is a special case and doesn't behave like a normal std::vector
	// So we have to add this special case to handle it
	// See https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0174r0.html#3.1
	class CLASS() BoolList : public std::vector<bool>
	{
		#ifndef KODGEN_PARSING
		Serialization_BoolList_GENERATED
		#endif
	};

	template <class T>
	class CLASS() List : public std::vector<T>
	{
	public:
		METHOD()
		std::size_t size() const noexcept
		{
			return std::vector<T>::size();
		}

		METHOD()
		void resize(const std::size_t& newSize) noexcept
		{
			std::vector<T>::resize(newSize);
		}

		METHOD()
		const void* getRawElement(const std::size_t& index) const
		{
			if (index >= 0 && index < size())
			{
				const T* data = std::vector<T>::data();
				const T* rawElement = data + index;

				return rawElement;
			}

			return nullptr;
		}

		METHOD()
		void* getRawElementMutable(const std::size_t& index)
		{
			return const_cast<void *>(getRawElement(index));
		}

		#ifndef KODGEN_PARSING
		Serialization_List_GENERATED
		#endif
	};
};

#ifndef KODGEN_PARSING
File_SerializableList_GENERATED
#endif