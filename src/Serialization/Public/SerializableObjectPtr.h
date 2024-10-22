#pragma once

#ifndef KODGEN_PARSING
#include "SerializableObjectPtr.rfkh.h"
#include "Refureku/Refureku.h"
#endif

#include <memory>

namespace rfk
{
	class Struct;
};

namespace Serialization NAMESPACE()
{
	template <class t_base_class>
	class CLASS() ObjectPtr
	{
	public:
		ObjectPtr()= default;

		#ifndef KODGEN_PARSING
		template <class t_derived_class>
		ObjectPtr(std::shared_ptr<t_derived_class>& objectPtr)
		{
			setSharedPointer(objectPtr);
		}
		#endif

		std::shared_ptr<t_base_class> getSharedPointer() const
		{
			return m_objectPtr;
		}

		#ifndef KODGEN_PARSING
		template <class t_derived_class>
		void setSharedPointer(std::shared_ptr<t_derived_class>& objectPtr)
		{
			m_runtimeClassId = t_derived_class::staticGetArchetype().getId();
			m_objectPtr = std::move(objectPtr);
		}
		#endif

		METHOD()
		std::size_t getRuntimeClassId() const
		{
			return m_runtimeClassId;
		}

		METHOD()
		const void* getRawPtr() const
		{
			return m_objectPtr.get();
		}

		METHOD()
		void* allocate(const std::size_t&& classId)
		{
		#ifndef KODGEN_PARSING
			rfk::Struct const* objectClass = rfk::getDatabase().getStructById(classId);

			if (objectClass != nullptr)
			{
				m_objectPtr= objectClass->makeSharedInstance<t_base_class>();
				m_runtimeClassId = classId;

				return m_objectPtr.get();
			}
		#endif

			return nullptr;
		}

		#ifndef KODGEN_PARSING
		Serialization_ObjectPtr_GENERATED
		#endif

	private:
		std::size_t m_runtimeClassId = 0;
		std::shared_ptr<t_base_class> m_objectPtr;
	};
};

#ifndef KODGEN_PARSING
File_SerializableObjectPtr_GENERATED
#endif