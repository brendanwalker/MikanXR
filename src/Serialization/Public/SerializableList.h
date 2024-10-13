#pragma once

#include "SerializationExport.h"
#include "SerializableList.rfkh.h"

#include "Refureku/Containers/Vector.h"

namespace Serialization NAMESPACE()
{
	template <typename T>
	class CLASS() List : public rfk::Vector<T>
	{
	public:
		METHOD()
		std::size_t size() const noexcept
		{
			return rfk::Vector<T>::size();
		}

		METHOD()
		void resize(const std::size_t& newSize) noexcept
		{
			return rfk::Vector<T>::resize(newSize);
		}

		METHOD()
		const void* getRawElement(const std::size_t& index) const
		{
			if (index >= 0 && index < size())
			{
				const T* rawArray = rfk::Vector<T>::data();

				return rawArray + index;
			}

			return nullptr;
		}

		METHOD()
		void* getRawElementMutable(const std::size_t& index)
		{
			return const_cast<void *>(getRawElement(index));
		}
	Serialization_List_GENERATED
	};
};

File_SerializableList_GENERATED