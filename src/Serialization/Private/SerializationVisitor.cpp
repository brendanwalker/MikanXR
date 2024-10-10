#include "SerializationVisitor.h"
#include "SerializationUtility.h"

#include <Refureku/Refureku.h>


namespace Serialization
{
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

		const rfk::EEntityKind fieldKind = field.getKind();
		rfk::Type const& fieldType = field.getType();
		rfk::Archetype const* fieldArchetype = fieldType.getArchetype();
		rfk::EEntityKind fieldArchetypeKind = fieldArchetype ? fieldArchetype->getKind() : rfk::EEntityKind::Undefined;
		const char* fieldArchetypeName = fieldArchetype ? fieldArchetype->getName() : "";

		if (fieldArchetypeKind == rfk::EEntityKind::Class)
		{
			rfk::Class const* classType = rfk::classCast(fieldArchetype);

			if (classType != nullptr)
			{
				void* classInstance = field.getPtrUnsafe(instance);

				visitor->visitClass(classInstance, field, *classType);
			}
			else
			{
				throw std::runtime_error(stringify("Field ", field.getName(), " was not an class type"));
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Struct)
		{
			rfk::Struct const* structType = rfk::structCast(fieldArchetype);

			if (structType != nullptr)
			{
				void* structInstance = field.getPtrUnsafe(instance);

				visitor->visitStruct(structInstance, field, *structType);
			}
			else
			{
				throw std::runtime_error(stringify("Field ", field.getName(), " was not a struct type"));
			}
		}
		else if (fieldArchetypeKind == rfk::EEntityKind::Enum)
		{
			rfk::Enum const* enumType = rfk::enumCast(fieldArchetype);

			if (enumType != nullptr)
			{
				visitor->visitEnum(instance, field, *enumType);
			}
			else
			{
				throw std::runtime_error(stringify("Field ", field.getName(), " was not an enum type"));
			}
		}
		else if (fieldKind == rfk::EEntityKind::Field)
		{
			if (fieldType == rfk::getType<bool>())
			{
				visitor->visitBool(instance, field);
			}
			else if (fieldType == rfk::getType<uint8_t>())
			{
				visitor->visitByte(instance, field);
			}
			else if (fieldType == rfk::getType<int8_t>())
			{
				visitor->VisitUByte(instance, field);
			}
			else if (fieldType == rfk::getType<uint16_t>())
			{
				visitor->visitUShort(instance, field);
			}
			else if (fieldType == rfk::getType<int16_t>())
			{
				visitor->visitShort(instance, field);
			}
			else if (fieldType == rfk::getType<uint32_t>())
			{
				visitor->visitUInt(instance, field);
			}
			else if (fieldType == rfk::getType<int32_t>())
			{
				visitor->visitInt(instance, field);
			}
			else if (fieldType == rfk::getType<uint64_t>())
			{
				visitor->visitULong(instance, field);
			}
			else if (fieldType == rfk::getType<int64_t>())
			{
				visitor->visitLong(instance, field);
			}
			else if (fieldType == rfk::getType<float>())
			{
				visitor->visitFloat(instance, field);
			}
			else if (fieldType == rfk::getType<double>())
			{
				visitor->visitDouble(instance, field);
			}
			else if (fieldType == rfk::getType<std::string>())
			{
				visitor->visitString(instance, field);
			}
			else
			{
				throw std::runtime_error(stringify("Field ", field.getName(), " has unsupported type"));
			}
		}
		else
		{
			throw std::runtime_error(
				stringify("Field ", field.getName(), " has unsupported archetype kind ", (int)fieldKind));
		}
	}
}