#include "ServerEntitySerializer.h"
#include "SerializableList.h"
#include "SerializableObjectPtr.h"
#include "SerializableString.h"
#include "SerializationProperty.h"
#include "StringUtils.h"
#include "Logger.h"

#include "Refureku/Refureku.h"

namespace Serialization
{
	class EntityAccessorReadVisitor : public IVisitor
	{
	public:
		EntityAccessorReadVisitor(IEntityAccessorConstPtr entityAccessor) : m_entityAccessor(entityAccessor) {}

		virtual void visitClass(ValueAccessor const& accessor) override
		{
			rfk::Type const& fieldType = accessor.getType();
			rfk::Class const* fieldClassType = accessor.getClassType();
			rfk::EClassKind classKind = fieldClassType->getClassKind();

			if (fieldType == rfk::getType<Serialization::BoolList>())
			{
				visitBoolList(accessor);
			}
			else if (classKind == rfk::EClassKind::TemplateInstantiation)
			{
				void* arrayInstance = accessor.getUntypedValueMutablePtr();
				const auto* templateClassInstanceType = rfk::classTemplateInstantiationCast(fieldClassType);
				std::string templateTypeName = templateClassInstanceType->getClassTemplate().getName();

				// See if the field is a Serialization::List<T>
				if (templateTypeName == "List" &&
					templateClassInstanceType->getTemplateArgumentsCount() == 1)
				{
					// Get the type of the elements in the array from the template argument
					auto const& templateArg =
						static_cast<rfk::TypeTemplateArgument const&>(
							templateClassInstanceType->getTemplateArgumentAt(0));
					rfk::Type const& elementType = templateArg.getType();

					if (elementType == rfk::getType<int>())
					{
						visitIntList(accessor);
					}
					else
					{
						throw std::runtime_error(
							"EntityAccessorReadVisitor::visitClass() List with unsupported element type");
					}
				}
				else
				{
					throw std::runtime_error(
						StringUtils::stringify("EntityAccessorReadVisitor::visitClass() ",
							"Template Class Type ", templateTypeName, " is not supported"));
				}
			}
			else if (fieldType == rfk::getType<PolymorphicObjectPtr>())
			{
				visitObjectPtr(accessor);
			}
			else if (fieldType == rfk::getType<Serialization::String>())
			{
				visitString(accessor);
			}
			else
			{
				EntityAccessorReadVisitor::visitStruct(accessor);
			}
		}

		void visitObjectPtr(ValueAccessor const& accessor)
		{
			// Get the shared pointer we are writing 
			PolymorphicObjectPtr& objPtrInstance = accessor.getTypedValueMutableRef<PolymorphicObjectPtr>();

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::POLYMORPHIC_OBJECT)
			{
				// Directly use the PolymorphicObjectPtr from the source property value
				objPtrInstance = sourcePropertyValue.getPolymorphicObjectValue();
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitObjectPtr() ",
						"Missing valid PolymorphicObjectPtr for", accessor.getName()));
			}
		}

		void visitBoolList(ValueAccessor const& arrayAccessor)
		{
			auto& boolListWrapper = arrayAccessor.getTypedValueMutableRef<Serialization::BoolList>();
			auto& boolList = boolListWrapper.getVectorMutable();

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::BOOL_ARRAY)
			{
				boolList = sourcePropertyValue.getBoolArrayValue();
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitObjectPtr() ",
						"Missing valid BoolList for", arrayAccessor.getName()));
			}
		}

		void visitIntList(ValueAccessor const& arrayAccessor)
		{
			auto& destArray = arrayAccessor.getTypedValueMutableRef<Serialization::List<int>>();

			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(arrayAccessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT_ARRAY)
			{
				const std::vector<int>& sourceArray = sourcePropertyValue.getIntArrayValue();

				destArray.assign(sourceArray.begin(), sourceArray.end());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitObjectPtr() ",
						"Missing valid IntList for", arrayAccessor.getName()));
			}
		}

		void visitString(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::STRING)
			{
				auto* variablePtr = accessor.getTypedValueMutablePtr<Serialization::String>();

				variablePtr->setValue(sourcePropertyValue.getStringValue());
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitObjectPtr() ",
						"Missing valid String for", accessor.getName()));
			}
		}

		virtual void visitStruct(ValueAccessor const& accessor) override
		{
			void* childObjectInstance = accessor.getUntypedValueMutablePtr();
			rfk::Struct const* structType = accessor.getStructType();
			EntityAccessorReadVisitor jsonVisitor(m_entityAccessor);

			Serialization::visitStruct(childObjectInstance, *structType, &jsonVisitor);
		}

		virtual void visitEnum(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				rfk::Enum const& enumType = *accessor.getEnumType();
				rfk::Archetype const& enumArchetype = enumType.getUnderlyingArchetype();
				rfk::EnumValue const* enumValue = nullptr;
				int enumIntValue = accessor.getValue<int>();

				enumValue = enumType.getEnumValue(enumIntValue);

				if (enumValue != nullptr)
				{
					void* enumInstance = accessor.getInstanceMutable();
					rfk::Field const* enumField = accessor.getField();
					const int64_t enumInt64Value = enumValue->getValue();

					if (enumField != nullptr)
					{
						enumField->setUnsafe(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
					}
					else
					{
						std::memcpy(enumInstance, &enumInt64Value, enumArchetype.getMemorySize());
					}
				}
				else
				{
					throw std::runtime_error(
						StringUtils::stringify("EntityAccessorReadVisitor::visitEnum() ",
							"Enum Accessor ", accessor.getName(),
							" has an invalid value ", enumIntValue));
				}
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitEnum() ",
						"Missing valid Enum for", accessor.getName()));
			}
		}

		virtual void visitBool(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::BOOL)
			{
				bool value = sourcePropertyValue.getBoolValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitBool() ",
						"Bool Accessor ", accessor.getName(),
						" was not a bool value"));
			}
		}

		virtual void visitByte(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int8_t value = (int8_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitByte() ",
						"Byte Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUByte(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint8_t value = (uint8_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUByte() ",
						"UByte Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitShort(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int16_t value = (int16_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitShort() ",
						"Short Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUShort(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint16_t value = (uint16_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUShort() ",
						"UShort Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitInt(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				int32_t value = (int32_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitInt() ",
						"Int32 Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitUInt(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::INT)
			{
				uint32_t value = (uint32_t)sourcePropertyValue.getIntValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitUInt() ",
						"UInt32 Accessor ", accessor.getName(),
						" was not a integer value"));
			}
		}

		virtual void visitLong(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::LONG)
			{
				int64_t value = (int64_t)sourcePropertyValue.getLongValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitLong() ",
						"Int64 Accessor ", accessor.getName(),
						" was not a long value"));
			}
		}

		virtual void visitULong(ValueAccessor const& accessor)
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::LONG)
			{
				uint64_t value = (uint64_t)sourcePropertyValue.getLongValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitULong() ",
						"UInt64 Accessor ", accessor.getName(),
						" was not a long value"));
			}
		}

		virtual void visitFloat(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::FLOAT)
			{
				float value = sourcePropertyValue.getFloatValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitFloat() ",
						"Float Accessor ", accessor.getName(),
						" was not a float value"));
			}
		}

		virtual void visitDouble(ValueAccessor const& accessor) override
		{
			MikanVariant sourcePropertyValue;
			if (m_entityAccessor->getPropertyValue(accessor.getName(), sourcePropertyValue) &&
				sourcePropertyValue.value_type == MikanVariantType::DOUBLE)
			{
				double value = sourcePropertyValue.getDoubleValue();

				accessor.setValueByType(value);
			}
			else
			{
				throw std::runtime_error(
					StringUtils::stringify("EntityAccessorReadVisitor::visitDouble() ",
						"Double Accessor ", accessor.getName(),
						" was not a double value"));
			}
		}

	private:
		IEntityAccessorConstPtr m_entityAccessor;
	};

	// Public API
	bool serializeFromEntity(IEntityAccessorConstPtr entityAccessor, void* instance, rfk::Struct const& structType)
	{
		try
		{
			EntityAccessorReadVisitor visitor(entityAccessor);
			Serialization::visitStruct(instance, structType, &visitor);

			return true;
		}
		catch (std::runtime_error* e)
		{
			MIKAN_LOG_ERROR("Serialization::serializeFromEntity") << e->what();
			return false;
		}
	}
};