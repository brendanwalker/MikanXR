#pragma once

#include <string>
#include "assert.h"

namespace rfk
{
	class Archetype;
	class Enum;
	class Field;
	class Struct;
	using Class = Struct;
	class Type;
};

namespace Serialization
{
	class ValueAccessor
	
	{
	public:
		ValueAccessor(const void* instance, rfk::Field const& field);
		ValueAccessor(const void* instance, rfk::Type const& type);
		ValueAccessor(void* instance, rfk::Field const& field);
		ValueAccessor(void* instance, rfk::Type const& type);

		const void* getInstance() const;
		void* getInstanceMutable() const;

		rfk::Field const* getField() const { return m_field; }
		rfk::Type const& getType() const { return m_type; }
		std::string const& getName() const { return m_name; }

		rfk::Class const* getClassType() const;
		rfk::Struct const* getStructType() const;
		rfk::Enum const* getEnumType() const;

		const void* getUntypedValuePtr() const;
		void* getUntypedValueMutablePtr() const;

		template <typename t_value_type>
		const t_value_type& getTypedValueRef() const
		{
			return *getTypedValuePtr<t_value_type>();
		}

		template <typename t_value_type>
		t_value_type& getTypedValueMutableRef() const
		{
			return *getTypedValueMutablePtr<t_value_type>();
		}

		template <typename t_value_type>
		const t_value_type* getTypedValuePtr() const
		{
			assert(m_type == rfk::getType<t_value_type>());

			return reinterpret_cast<const t_value_type*>(getUntypedValuePtr());
		}

		template <typename t_value_type>
		t_value_type* getTypedValueMutablePtr() const
		{
			assert(m_type == rfk::getType<t_value_type>());

			return reinterpret_cast<t_value_type*>(getUntypedValueMutablePtr());
		}

		template <typename t_value_type>
		t_value_type getValue() const
		{
			assert(m_type == rfk::getType<t_value_type>());
			assert(m_type.isValue());
			assert(m_instance != nullptr);

			if (m_field)
			{
				return m_field->getUnsafe<t_value_type>(m_instance);
			}
			else
			{
				return *reinterpret_cast<t_value_type*>(m_instance);
			}
		}

		template <typename t_value_type>
		void setValueByType(const t_value_type& value) const
		{
#ifdef _DEBUG
			auto const* templateArchetype= rfk::getType<t_value_type>().getArchetype();
			char const*	templateTypeName= templateArchetype ? templateArchetype->getName() : "";
			auto const* accessorArchetype= m_type.getArchetype();
		 	char const* accessorTypeName= accessorArchetype ? accessorArchetype->getName() : "";
#endif

			assert(rfk::getType<t_value_type>().match(m_type));
			assert(m_type.isValue());
			assert(m_instance != nullptr);

			if (m_field)
			{
				m_field->setUnsafe(m_instance, &value, sizeof(t_value_type));
			}
			else
			{
				*reinterpret_cast<t_value_type*>(m_instance) = value;
			}
		}

	private:
		void* m_instance;
		bool m_isConst;
		rfk::Field const* m_field;
		rfk::Type const& m_type;
		std::string m_name;
	};

	class IVisitor
	{
	public:
		virtual void visitClass(ValueAccessor const& accessor) { }
		virtual void visitStruct(ValueAccessor const& accessor) { }
		virtual void visitEnum(ValueAccessor const& accessor) { }
		virtual void visitBool(ValueAccessor const& accessor) { }
		virtual void visitByte(ValueAccessor const& accessor) { }
		virtual void visitUByte(ValueAccessor const& accessor) { }
		virtual void visitShort(ValueAccessor const& accessor) { }
		virtual void visitUShort(ValueAccessor const& accessor) { }
		virtual void visitInt(ValueAccessor const& accessor) { }
		virtual void visitUInt(ValueAccessor const& accessor) { }
		virtual void visitLong(ValueAccessor const& accessor) { }
		virtual void visitULong(ValueAccessor const& accessor) { }
		virtual void visitFloat(ValueAccessor const& accessor) { }
		virtual void visitDouble(ValueAccessor const& accessor) { }
		virtual void visitString(ValueAccessor const& accessor) { }
	};

	template <typename t_struct_type>
	void visitStruct(t_struct_type& instance, IVisitor *visitor)
	{
		visitStruct(&instance, t_struct_type::staticGetArchetype(), visitor, userdata);
	}

	template <typename t_struct_type>
	void visitStruct(const t_struct_type& instance, IVisitor* visitor)
	{
		visitStruct(&instance, t_struct_type::staticGetArchetype(), visitor, userdata);
	}

	void visitStruct(const void* instance, rfk::Struct const& structType, IVisitor *visitor);
	void visitStruct(void* instance, rfk::Struct const& structType, IVisitor *visitor);

	void visitField(const void* instance, rfk::Field const& fieldType, IVisitor *visitor);
	void visitField(void* instance, rfk::Field const& fieldType, IVisitor *visitor);

	void visitValue(ValueAccessor const& accessor, IVisitor *visitor);
};