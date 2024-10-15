#include "SerializationVisitor.h"
#include "SerializationUtility.h"

#include <Refureku/Refureku.h>


namespace Serialization
{
	ValueAccessor::ValueAccessor(const void* instance, rfk::Field const& field) :
		m_instance(const_cast<void *>(instance)),
		m_isConst(true),
		m_field(&field),
		m_type(field.getType()),
		m_name(field.getName())
	{}

	ValueAccessor::ValueAccessor(const void* instance, rfk::Type const& type) :
		m_instance(const_cast<void *>(instance)),
		m_isConst(true),
		m_field(nullptr),
		m_type(type),
		m_name(type.getArchetype()->getName())
	{}

	ValueAccessor::ValueAccessor(void* instance, rfk::Field const& field) :
		m_instance(instance),
		m_isConst(false),
		m_field(&field),
		m_type(field.getType()),
		m_name(field.getName())
	{}

	ValueAccessor::ValueAccessor(void* instance, rfk::Type const& type) :
		m_instance(instance),
		m_isConst(false),
		m_field(nullptr),
		m_type(type),
		m_name(type.getArchetype()->getName())
	{}

	const void* ValueAccessor::getInstance() const
	{ 
		return m_instance; 
	}

	void* ValueAccessor::getInstanceMutable() const 
	{ 
		assert(!m_isConst);
		return m_instance; 
	}

	rfk::Class const* ValueAccessor::getClassType() const
	{
		return rfk::classCast(m_type.getArchetype());
	}

	rfk::Struct const* ValueAccessor::getStructType() const
	{
		return rfk::structCast(m_type.getArchetype());
	}

	rfk::Enum const* ValueAccessor::getEnumType() const
	{
		return rfk::enumCast(m_type.getArchetype());
	}

	const void* ValueAccessor::getUntypedValuePtr() const
	{
		assert(m_type.isValue());
		assert(m_instance != nullptr);

		if (m_field)
		{
			return m_field->getPtrUnsafe(m_instance);
		}
		else
		{
			return m_instance;
		}
	}

	void* ValueAccessor::getUntypedValueMutablePtr() const
	{
		assert(!m_isConst);
		return const_cast<void*>(getUntypedValuePtr());
	}

	struct VisitorUserdata
	{
		void* instance;
		IVisitor *visitor;
	};

	void visitStruct(void* instance, rfk::Struct const& structType, IVisitor *visitor)
	{
		VisitorUserdata userdata= {instance, visitor};

		structType.foreachField(
			[](rfk::Field const& field, void* userdata) -> bool {
				auto* args = reinterpret_cast<VisitorUserdata*>(userdata);

				// Skip this field is it is non-public or is static
				if (field.getAccess() != rfk::EAccessSpecifier::Public || field.isStatic())
				{
					return true;
				}

				Serialization::visitField(args->instance, field, args->visitor);
				return true;
			},
			&userdata,
			true);
	}

	void visitField(void* instance, rfk::Field const& field, IVisitor *visitor)
	{
		// Error if this field is non-public or is static
		if (field.getAccess() != rfk::EAccessSpecifier::Public)
		{
			throw std::runtime_error(stringify("Field ", field.getName(), " was not public"));
		}
		if (field.isStatic())
		{
			throw std::runtime_error(stringify("Field ", field.getName(), " is static"));
		}
		
		visitValue(ValueAccessor(instance, field), visitor);
	}

	void visitValue(ValueAccessor const& accessor, IVisitor* visitor)
	{
		rfk::Type const& fieldType = accessor.getType();
		rfk::Archetype const* fieldArchetype = fieldType.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = fieldArchetype ? fieldArchetype->getKind() : rfk::EEntityKind::Undefined;
		const char* fieldArchetypeName = fieldArchetype ? fieldArchetype->getName() : "";
		const std::string& fieldName = accessor.getName();

		if (fieldArchetypeKind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(fieldArchetype);

			if (classType != nullptr)
			{
				visitor->visitClass(accessor);
			}
			else
			{
				throw std::runtime_error(stringify("Accessor ", accessor.getName(), " was not an class type"));
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(fieldArchetype);

			if (structType != nullptr)
			{
				visitor->visitStruct(accessor);
			}
			else
			{
				throw std::runtime_error(stringify("Accessor ", accessor.getName(), " was not a struct type"));
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(fieldArchetype);

			if (enumType != nullptr)
			{
				visitor->visitEnum(accessor);
			}
			else
			{
				throw std::runtime_error(stringify("Accessor ", accessor.getName(), " was not an enum type"));
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::FundamentalArchetype)
		{
			if (fieldType == rfk::getType<bool>())
			{
				visitor->visitBool(accessor);
			}
			else if (fieldType == rfk::getType<uint8_t>())
			{
				visitor->VisitUByte(accessor);
			}
			else if (fieldType == rfk::getType<int8_t>())
			{
				visitor->visitByte(accessor);
			}
			else if (fieldType == rfk::getType<uint16_t>())
			{
				visitor->visitUShort(accessor);
			}
			else if (fieldType == rfk::getType<int16_t>())
			{
				visitor->visitShort(accessor);
			}
			else if (fieldType == rfk::getType<uint32_t>())
			{
				visitor->visitUInt(accessor);
			}
			else if (fieldType == rfk::getType<int32_t>())
			{
				visitor->visitInt(accessor);
			}
			else if (fieldType == rfk::getType<uint64_t>())
			{
				visitor->visitULong(accessor);
			}
			else if (fieldType == rfk::getType<int64_t>())
			{
				visitor->visitLong(accessor);
			}
			else if (fieldType == rfk::getType<float>())
			{
				visitor->visitFloat(accessor);
			}
			else if (fieldType == rfk::getType<double>())
			{
				visitor->visitDouble(accessor);
			}
			else
			{
				throw std::runtime_error(stringify("Accessor ", accessor.getName(), " has unsupported type"));
			}
		}
		else if (fieldType == rfk::getType<std::string>())
		{
			visitor->visitString(accessor);
		}
		else
		{
			throw std::runtime_error(stringify("Unsupported archetype kind ", (int)fieldArchetypeKind));
		}
	}
}