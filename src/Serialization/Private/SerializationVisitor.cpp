#include "SerializationVisitor.h"
#include "SerializationUtility.h"

#include <Refureku/Refureku.h>


namespace Serialization
{
	struct AccessorData
	{
		void* instance;
		bool isConst;
		rfk::Field const* field;
		rfk::Type const& type;
		std::string name;

		AccessorData(const void* instance, rfk::Field const& field) :
			instance(const_cast<void*>(instance)),
			isConst(true),
			field(&field),
			type(field.getType()),
			name(field.getName())
		{}

		AccessorData(const void* instance, rfk::Type const& type) :
			instance(const_cast<void*>(instance)),
			isConst(true),
			field(nullptr),
			type(type),
			name(type.getArchetype()->getName())
		{}

		AccessorData(void* instance, rfk::Field const& field) :
			instance(instance),
			isConst(false),
			field(&field),
			type(field.getType()),
			name(field.getName())
		{}

		AccessorData(void* instance, rfk::Type const& type) :
			instance(instance),
			isConst(false),
			field(nullptr),
			type(type),
			name(type.getArchetype()->getName())
		{}

		AccessorData(const AccessorData& other) :
			instance(other.instance),
			isConst(other.isConst),
			field(other.field),
			type(other.type),
			name(other.name)
		{}

		AccessorData(AccessorData&& other) :
			instance(other.instance),
			isConst(other.isConst),
			field(other.field),
			type(other.type),
			name(other.name)
		{}
	};

	ValueAccessor::ValueAccessor(const void* instance, rfk::Field const& field) :
		m_pimpl(new AccessorData(instance, field))
	{}

	ValueAccessor::ValueAccessor(const void* instance, rfk::Type const& type) :
		m_pimpl(new AccessorData(instance, type))
	{}

	ValueAccessor::ValueAccessor(void* instance, rfk::Field const& field) :
		m_pimpl(new AccessorData(instance, field))
	{}

	ValueAccessor::ValueAccessor(void* instance, rfk::Type const& type) :
		m_pimpl(new AccessorData(instance, type))
	{}

	ValueAccessor::ValueAccessor(const ValueAccessor& other) :
		m_pimpl(new AccessorData(*other.m_pimpl))
	{ }

	ValueAccessor::ValueAccessor(ValueAccessor&& other) :
		m_pimpl(new AccessorData(*other.m_pimpl))
	{ }

	ValueAccessor::~ValueAccessor()
	{
		delete m_pimpl;
	}

	ValueAccessor& ValueAccessor::operator=(const ValueAccessor& other)
	{
		if (this != &other)
		{
			delete m_pimpl;
			m_pimpl = new AccessorData(*other.m_pimpl);
		}

		return *this;
	}

	const void* ValueAccessor::getInstance() const
	{ 
		return m_pimpl->instance; 
	}

	void* ValueAccessor::getInstanceMutable() const 
	{ 
		assert(!m_pimpl->isConst);
		return m_pimpl->instance; 
	}

	rfk::Field const* ValueAccessor::getField() const 
	{
		return m_pimpl->field; 
	}

	rfk::Type const& ValueAccessor::getType() const
	{
		return m_pimpl->type; 
	}

	std::string const& ValueAccessor::getName() const
	{
		return m_pimpl->name; 
	}

	rfk::Class const* ValueAccessor::getClassType() const
	{
		return rfk::classCast(m_pimpl->type.getArchetype());
	}

	rfk::Struct const* ValueAccessor::getStructType() const
	{
		return rfk::structCast(m_pimpl->type.getArchetype());
	}

	rfk::Enum const* ValueAccessor::getEnumType() const
	{
		return rfk::enumCast(m_pimpl->type.getArchetype());
	}

	const void* ValueAccessor::getUntypedValuePtr() const
	{
		assert(m_pimpl->type.isValue());
		assert(m_pimpl->instance != nullptr);

		if (m_pimpl->field)
		{
			return m_pimpl->field->getPtrUnsafe(m_pimpl->instance);
		}
		else
		{
			return m_pimpl->instance;
		}
	}

	void* ValueAccessor::getUntypedValueMutablePtr() const
	{
		assert(!m_pimpl->isConst);
		return const_cast<void*>(getUntypedValuePtr());
	}

	struct ConstVisitorUserdata
	{
		const void* instance;
		IVisitor *visitor;
	};

	void visitStruct(const void* instance, rfk::Struct const& structType, IVisitor *visitor)
	{
		ConstVisitorUserdata userdata = {instance, visitor};

		structType.foreachField(
			[](rfk::Field const& field, void* userdata) -> bool {
			auto* args = reinterpret_cast<ConstVisitorUserdata*>(userdata);

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

	struct VisitorUserdata
	{
		void* instance;
		IVisitor* visitor;
	};

	void visitStruct(void* instance, rfk::Struct const& structType, IVisitor* visitor)
	{
		VisitorUserdata userdata = {instance, visitor};

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
		& userdata,
		true);
	}

	void visitField(const void* instance, rfk::Field const& field, IVisitor* visitor)
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
				visitor->visitUByte(accessor);
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
		else
		{
			throw std::runtime_error(stringify("Unsupported archetype kind ", (int)fieldArchetypeKind));
		}
	}
}